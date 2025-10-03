#ifndef _VIDEODEV2_H
#define _VIDEODEV2_H

// V4L2 header stub for cross-compilation
// Real V4L2 headers will be available on Raspberry Pi

#include <stdint.h>
#include <sys/ioctl.h>

// V4L2 buffer types
enum v4l2_buf_type {
    V4L2_BUF_TYPE_VIDEO_CAPTURE = 1,
    V4L2_BUF_TYPE_VIDEO_OUTPUT = 2,
    V4L2_BUF_TYPE_VIDEO_OVERLAY = 3,
    V4L2_BUF_TYPE_VBI_CAPTURE = 4,
    V4L2_BUF_TYPE_VBI_OUTPUT = 5,
    V4L2_BUF_TYPE_SLICED_VBI_CAPTURE = 6,
    V4L2_BUF_TYPE_SLICED_VBI_OUTPUT = 7,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
    V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE = 10,
    V4L2_BUF_TYPE_SDR_CAPTURE = 11,
    V4L2_BUF_TYPE_SDR_OUTPUT = 12,
    V4L2_BUF_TYPE_META_CAPTURE = 13,
    V4L2_BUF_TYPE_META_OUTPUT = 14,
    V4L2_BUF_TYPE_PRIVATE = 0x80,
};

// V4L2 memory types
enum v4l2_memory {
    V4L2_MEMORY_MMAP = 1,
    V4L2_MEMORY_USERPTR = 2,
    V4L2_MEMORY_OVERLAY = 3,
    V4L2_MEMORY_DMABUF = 4,
};

// V4L2 field types
enum v4l2_field {
    V4L2_FIELD_ANY = 0,
    V4L2_FIELD_NONE = 1,
    V4L2_FIELD_TOP = 2,
    V4L2_FIELD_BOTTOM = 3,
    V4L2_FIELD_INTERLACED = 4,
    V4L2_FIELD_SEQ_TB = 5,
    V4L2_FIELD_SEQ_BT = 6,
    V4L2_FIELD_ALTERNATE = 7,
    V4L2_FIELD_INTERLACED_TB = 8,
    V4L2_FIELD_INTERLACED_BT = 9,
};

// V4L2 pixel formats
#define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4')
#define V4L2_PIX_FMT_MJPEG    v4l2_fourcc('M', 'J', 'P', 'G')
#define V4L2_PIX_FMT_YUV420   v4l2_fourcc('Y', 'U', '1', '2')

// V4L2 fourcc helper
static inline uint32_t v4l2_fourcc(char a, char b, char c, char d) {
    return ((uint32_t)a) | (((uint32_t)b) << 8) | (((uint32_t)c) << 16) | (((uint32_t)d) << 24);
}

// V4L2 format structure
struct v4l2_pix_format {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t field;
    uint32_t bytesperline;
    uint32_t sizeimage;
    uint32_t colorspace;
    uint32_t priv;
    uint32_t flags;
    union {
        uint32_t ycbcr_enc;
        uint32_t hsv_enc;
    };
    uint32_t quantization;
    uint32_t xfer_func;
};

struct v4l2_format {
    uint32_t type;
    union {
        struct v4l2_pix_format pix;
        uint8_t raw_data[200];
    } fmt;
};

// V4L2 buffer structure
struct v4l2_buffer {
    uint32_t index;
    uint32_t type;
    uint32_t bytesused;
    uint32_t flags;
    uint32_t field;
    struct timeval timestamp;
    uint32_t sequence;
    uint32_t memory;
    union {
        uint32_t offset;
        unsigned long userptr;
        int32_t fd;
    } m;
    uint32_t length;
    uint32_t reserved2;
};

// V4L2 request buffers structure
struct v4l2_requestbuffers {
    uint32_t count;
    uint32_t type;
    uint32_t memory;
    uint32_t reserved[2];
};

// V4L2 timeval structure (use system timeval if available)
#ifndef _STRUCT_TIMEVAL
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

// V4L2 timecode structure
struct v4l2_timecode {
    uint32_t type;
    uint32_t flags;
    uint8_t frames;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t userbits[4];
};

// V4L2 IOCTL commands
#define VIDIOC_REQBUFS        _IOWR('V', 8, struct v4l2_requestbuffers)
#define VIDIOC_QUERYBUF       _IOWR('V', 9, struct v4l2_buffer)
#define VIDIOC_QBUF           _IOWR('V', 15, struct v4l2_buffer)
#define VIDIOC_DQBUF          _IOWR('V', 17, struct v4l2_buffer)
#define VIDIOC_STREAMON        _IOW('V', 18, int)
#define VIDIOC_STREAMOFF       _IOW('V', 19, int)
#define VIDIOC_S_FMT           _IOWR('V', 5, struct v4l2_format)
#define VIDIOC_G_FMT           _IOWR('V', 4, struct v4l2_format)

// V4L2 buffer flags
#define V4L2_BUF_FLAG_MAPPED          0x00000001
#define V4L2_BUF_FLAG_QUEUED          0x00000002
#define V4L2_BUF_FLAG_DONE            0x00000004
#define V4L2_BUF_FLAG_KEYFRAME        0x00000008
#define V4L2_BUF_FLAG_PFRAME          0x00000010
#define V4L2_BUF_FLAG_BFRAME          0x00000020
#define V4L2_BUF_FLAG_ERROR           0x00000040
#define V4L2_BUF_FLAG_TIMECODE        0x00000080
#define V4L2_BUF_FLAG_PREPARED        0x00000100
#define V4L2_BUF_FLAG_NO_CACHE_INVALIDATE 0x00000200
#define V4L2_BUF_FLAG_NO_CACHE_CLEAN  0x00000400
#define V4L2_BUF_FLAG_TIMESTAMP_MASK  0x0000e000
#define V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN 0x00000000
#define V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC 0x00002000
#define V4L2_BUF_FLAG_TIMESTAMP_COPY  0x00004000
#define V4L2_BUF_FLAG_TSTAMP_SRC_MASK 0x00070000
#define V4L2_BUF_FLAG_TSTAMP_SRC_EOF  0x00000000
#define V4L2_BUF_FLAG_TSTAMP_SRC_SOE  0x00010000
#define V4L2_BUF_FLAG_LAST            0x00080000

// V4L2 colorspace definitions
#define V4L2_COLORSPACE_DEFAULT       0
#define V4L2_COLORSPACE_SMPTE170M     1
#define V4L2_COLORSPACE_SMPTE240M     2
#define V4L2_COLORSPACE_REC709        3
#define V4L2_COLORSPACE_BT878         4
#define V4L2_COLORSPACE_470_SYSTEM_M  5
#define V4L2_COLORSPACE_470_SYSTEM_BG 6
#define V4L2_COLORSPACE_JPEG          7
#define V4L2_COLORSPACE_SRGB          8

#endif // _VIDEODEV2_H
