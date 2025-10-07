#include "h264_to_jpeg.h"
#include "h264_hw_decoder.h"
#include "mjpeg_hw_encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "videodev2.h"
static int video_fd = -1;
static void* buffers[4] = {NULL};
static int buffer_count = 0;
static bool running = true;

static void signal_handler(int sig) {
    printf("\n🛑 Received signal %d, shutting down gracefully...\n", sig);
    running = false;
}

static bool v4l2_set_format(int fd, int width, int height) {
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        printf("❌ Failed to set format: %s\n", strerror(errno));
        return false;
    }
    
    printf("✅ Format set: %dx%d H.264\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    return true;
}

static bool v4l2_request_buffers(int fd, int count) {
    struct v4l2_requestbuffers req = {0};
    req.count = count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        printf("❌ Failed to request buffers: %s\n", strerror(errno));
        return false;
    }
    
    if (req.count < count) {
        printf("⚠️  Warning: requested %d buffers, got %d\n", count, req.count);
    }
    
    buffer_count = req.count;
    printf("✅ Allocated %d buffers\n", buffer_count);
    return true;
}

static bool v4l2_map_buffers(int fd) {
    struct v4l2_buffer buf = {0};
    
    for (int i = 0; i < buffer_count; i++) {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            printf("❌ Failed to query buffer %d: %s\n", i, strerror(errno));
            return false;
        }
        
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i] == MAP_FAILED) {
            printf("❌ Failed to map buffer %d: %s\n", i, strerror(errno));
            return false;
        }
        
        printf("✅ Mapped buffer %d: %zu bytes\n", i, buf.length);
    }
    
    return true;
}

static bool v4l2_start_capture(int fd) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        printf("❌ Failed to start capture: %s\n", strerror(errno));
        return false;
    }
    
    printf("✅ Capture started\n");
    return true;
}

static bool v4l2_stop_capture(int fd) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        printf("❌ Failed to stop capture: %s\n", strerror(errno));
        return false;
    }
    
    printf("✅ Capture stopped\n");
    return true;
}

static bool is_idr_frame(const uint8_t* data, size_t size) {
    if (size < 5) return false;
    
    for (size_t i = 0; i < size - 4; i++) {
        if ((data[i] == 0x00 && data[i+1] == 0x00 && data[i+2] == 0x00 && data[i+3] == 0x01) ||
            (data[i] == 0x00 && data[i+1] == 0x00 && data[i+2] == 0x01)) {
            
            size_t nal_offset = (data[i+2] == 0x01) ? 3 : 4;
            if (i + nal_offset < size) {
                uint8_t nal_type = data[i + nal_offset] & 0x1F;
                if (nal_type == 5) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

static bool process_h264_frame(const uint8_t* h264_data, size_t h264_size, int frame_number) {
    printf("📸 Processing frame %d (%zu bytes)\n", frame_number, h264_size);
    
    if (!is_idr_frame(h264_data, h264_size)) {
        printf("⏭️  Skipping non-IDR frame %d\n", frame_number);
        return true;
    }
    
    printf("🎯 IDR frame detected! Converting to JPEG...\n");
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    if (h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 85)) {
        printf("✅ JPEG conversion successful: %zu bytes\n", jpeg_size);
        
        char filename[256];
        snprintf(filename, sizeof(filename), "tmp/frame_%d_idr.jpg", frame_number);
        
        FILE* file = fopen(filename, "wb");
        if (file) {
            fwrite(jpeg_data, 1, jpeg_size, file);
            fclose(file);
            printf("💾 Saved: %s\n", filename);
        } else {
            printf("❌ Failed to save %s\n", filename);
        }
        
        h264_to_jpeg_free(jpeg_data);
        return true;
    } else {
        printf("❌ JPEG conversion failed: %s\n", h264_to_jpeg_get_error());
        return false;
    }
}

static void capture_loop(int fd) {
    struct v4l2_buffer buf = {0};
    int frame_count = 0;
    int idr_count = 0;
    time_t start_time = time(NULL);
    
    printf("\n🎬 Starting capture loop...\n");
    printf("Press Ctrl+C to stop\n\n");
    
    while (running) {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            if (errno == EAGAIN) {
                usleep(10000);
                continue;
            }
            printf("❌ Failed to dequeue buffer: %s\n", strerror(errno));
            break;
        }
        
        if (buf.bytesused > 0) {
            frame_count++;
            const uint8_t* frame_data = (const uint8_t*)buffers[buf.index];
            
            if (process_h264_frame(frame_data, buf.bytesused, frame_count)) {
                if (is_idr_frame(frame_data, buf.bytesused)) {
                    idr_count++;
                }
            }
        }
        
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            printf("❌ Failed to requeue buffer: %s\n", strerror(errno));
            break;
        }
        
        if (frame_count % 30 == 0) {
            time_t current_time = time(NULL);
            long elapsed = current_time - start_time;
            if (elapsed > 0) {
                long fps = frame_count / elapsed;
                printf("📊 Stats: %d frames, %d IDR frames, %ld fps\n", 
                       frame_count, idr_count, fps);
            } else {
                printf("📊 Stats: %d frames, %d IDR frames, calculating...\n", 
                       frame_count, idr_count);
            }
        }
    }
    
    printf("\n📊 Final statistics:\n");
    printf("   Total frames: %d\n", frame_count);
    printf("   IDR frames: %d\n", idr_count);
    if (frame_count > 0) {
        long idr_percent = (100 * idr_count) / frame_count;
        printf("   IDR ratio: %ld%%\n", idr_percent);
    } else {
        printf("   IDR ratio: 0%%\n");
    }
}

static void cleanup(int fd) {
    printf("\n🧹 Cleaning up...\n");
    
    if (fd >= 0) {
        v4l2_stop_capture(fd);
        close(fd);
    }
    
    for (int i = 0; i < buffer_count; i++) {
        if (buffers[i] && buffers[i] != MAP_FAILED) {
            munmap(buffers[i], 0);
        }
    }
    
    printf("✅ Cleanup completed\n");
}

int main(int argc, char* argv[]) {
    const char* device = "/dev/video0";
    int width = 1280;
    int height = 720;
    
    if (argc > 1) {
        device = argv[1];
    }
    if (argc > 3) {
        width = atoi(argv[2]);
        height = atoi(argv[3]);
    }
    
    printf("🎥 V4L2 H.264 to JPEG Test\n");
    printf("==========================\n");
    printf("Device: %s\n", device);
    printf("Resolution: %dx%d\n", width, height);
    printf("\n");
    
    if (!h264_hw_decoder_available() || !mjpeg_hw_encoder_available()) {
        printf("❌ Hardware components not available on this system\n");
        printf("   This test requires a Raspberry Pi with VideoCore IV GPU\n");
        return 1;
    }
    
    printf("✅ Hardware components available\n");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    video_fd = open(device, O_RDWR | O_NONBLOCK);
    if (video_fd == -1) {
        printf("❌ Failed to open %s: %s\n", device, strerror(errno));
        return 1;
    }
    
    printf("✅ Opened %s\n", device);
    
    // Set up V4L2
    if (!v4l2_set_format(video_fd, width, height)) {
        cleanup(video_fd);
        return 1;
    }
    
    if (!v4l2_request_buffers(video_fd, 4)) {
        cleanup(video_fd);
        return 1;
    }
    
    if (!v4l2_map_buffers(video_fd)) {
        cleanup(video_fd);
        return 1;
    }
    
    if (!v4l2_start_capture(video_fd)) {
        cleanup(video_fd);
        return 1;
    }
    
    capture_loop(video_fd);
    
    cleanup(video_fd);
    
    printf("\n🎉 Test completed successfully!\n");
    printf("Check the 'tmp/' directory for generated JPEG files\n");
    
    return 0;
}
