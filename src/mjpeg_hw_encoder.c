#include "mjpeg_hw_encoder.h"
#include "h264_hw_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
static void output_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    mjpeg_hw_encoder_t* encoder = (mjpeg_hw_encoder_t*)port->userdata;
    
    if (encoder && buffer->length > 0) {
        encoder->current_buffer = buffer;
        encoder->frame_ready = true;
        vcos_semaphore_post(&encoder->output_semaphore);
    }
    
    mmal_buffer_header_release(buffer);
}

static void input_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    mmal_buffer_header_release(buffer);
}

static bool convert_yuv420_to_mmal(const yuv420_frame_t* yuv, MMAL_BUFFER_HEADER_T* buffer) {
    if (!yuv || !buffer || !buffer->data) {
        return false;
    }
    
    int width = yuv->width;
    int height = yuv->height;
    int y_size = width * height;
    int uv_size = y_size / 4;
    
    memcpy(buffer->data, yuv->y_plane, y_size);
    
    for (int i = 0; i < uv_size; i++) {
        buffer->data[y_size + i * 2] = yuv->u_plane[i];
        buffer->data[y_size + i * 2 + 1] = yuv->v_plane[i];
    }
    
    return true;
}
#endif
#endif

bool mjpeg_hw_encoder_init(mjpeg_hw_encoder_t* encoder, int quality) {
    if (!encoder) return false;
    
    memset(encoder, 0, sizeof(mjpeg_hw_encoder_t));
    
    if (quality < 1 || quality > 100) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Invalid quality value: %d (must be 1-100)", quality);
        return false;
    }
    
    encoder->quality = quality;
    
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    vcos_init();
    
    if (vcos_semaphore_create(&encoder->output_semaphore, "output_sem", 0) != VCOS_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to create semaphore");
        return false;
    }
    
    MMAL_STATUS_T status = mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder->encoder);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to create encoder component: %s", mmal_status_to_string(status));
        return false;
    }
    
    encoder->input_port = encoder->encoder->input[0];
    encoder->output_port = encoder->encoder->output[0];
    
    MMAL_ES_FORMAT_T* input_format = encoder->input_port->format;
    input_format->type = MMAL_ES_TYPE_VIDEO;
    input_format->encoding = MMAL_ENCODING_I420;
    input_format->es->video.width = 0;
    input_format->es->video.height = 0;
    
    MMAL_ES_FORMAT_T* output_format = encoder->output_port->format;
    output_format->type = MMAL_ES_TYPE_VIDEO;
    output_format->encoding = MMAL_ENCODING_JPEG;
    output_format->es->video.width = 0;
    output_format->es->video.height = 0;
    
    status = mmal_port_format_commit(encoder->input_port);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to commit input format: %s", mmal_status_to_string(status));
        return false;
    }
    
    status = mmal_port_format_commit(encoder->output_port);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to commit output format: %s", mmal_status_to_string(status));
        return false;
    }
    
    encoder->output_port->userdata = (struct MMAL_PORT_USERDATA_T*)encoder;
    status = mmal_port_enable(encoder->output_port, output_callback);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to enable output port: %s", mmal_status_to_string(status));
        return false;
    }
    
    encoder->input_port->userdata = (struct MMAL_PORT_USERDATA_T*)encoder;
    status = mmal_port_enable(encoder->input_port, input_callback);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to enable input port: %s", mmal_status_to_string(status));
        return false;
    }
    
    encoder->input_pool = mmal_port_pool_create(encoder->input_port, 1, 0);
    if (!encoder->input_pool) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to create input pool");
        return false;
    }
    
    encoder->output_pool = mmal_port_pool_create(encoder->output_port, 1, 0);
    if (!encoder->output_pool) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to create output pool");
        return false;
    }
    
    status = mmal_component_enable(encoder->encoder);
    if (status != MMAL_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to enable encoder: %s", mmal_status_to_string(status));
        return false;
    }
    
    encoder->component_ready = true;
#else
    encoder->hw_available = false;
    snprintf(encoder->error_message, sizeof(encoder->error_message), 
            "Hardware encoder not available on this system");
#endif
#endif
    
    return true;
}

void mjpeg_hw_encoder_cleanup(mjpeg_hw_encoder_t* encoder) {
    if (!encoder) return;
    
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    if (encoder->component_ready) {
        if (encoder->input_port) {
            mmal_port_disable(encoder->input_port);
        }
        if (encoder->output_port) {
            mmal_port_disable(encoder->output_port);
        }
        
        if (encoder->encoder) {
            mmal_component_disable(encoder->encoder);
        }
    }
    
    if (encoder->input_pool) {
        mmal_port_pool_destroy(encoder->input_port, encoder->input_pool);
    }
    if (encoder->output_pool) {
        mmal_port_pool_destroy(encoder->output_port, encoder->output_pool);
    }
    
    if (encoder->encoder) {
        mmal_component_destroy(encoder->encoder);
    }
    
    vcos_semaphore_delete(&encoder->output_semaphore);
#endif
#endif
    
    memset(encoder, 0, sizeof(mjpeg_hw_encoder_t));
}

bool mjpeg_hw_encoder_encode(mjpeg_hw_encoder_t* encoder,
                            const yuv420_frame_t* yuv_frame,
                            uint8_t** jpeg_data,
                            size_t* jpeg_size) {
    if (!encoder || !yuv_frame || !jpeg_data || !jpeg_size) {
        if (encoder) {
            snprintf(encoder->error_message, sizeof(encoder->error_message), 
                    "Invalid parameters");
        }
        return false;
    }
    
    if (!yuv_frame->y_plane || !yuv_frame->u_plane || !yuv_frame->v_plane) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Invalid YUV frame data");
        return false;
    }
    
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    if (!encoder->component_ready) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Hardware encoder not ready");
        return false;
    }
    
    encoder->frame_ready = false;
    
    MMAL_BUFFER_HEADER_T* buffer = mmal_queue_get(encoder->input_pool->queue);
    if (!buffer) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "No input buffer available");
        return false;
    }
    
    if (!convert_yuv420_to_mmal(yuv_frame, buffer)) {
        mmal_buffer_header_release(buffer);
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to convert YUV420 to MMAL format");
        return false;
    }
    
    buffer->length = yuv_frame->y_size + 2 * yuv_frame->uv_size;
    buffer->offset = 0;
    buffer->flags = MMAL_BUFFER_HEADER_FLAG_FRAME_END;
    
    MMAL_STATUS_T status = mmal_port_send_buffer(encoder->input_port, buffer);
    if (status != MMAL_SUCCESS) {
        mmal_buffer_header_release(buffer);
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to send buffer: %s", mmal_status_to_string(status));
        return false;
    }
    
    if (vcos_semaphore_wait_timeout(&encoder->output_semaphore, 1000) != VCOS_SUCCESS) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Timeout waiting for encoded frame");
        return false;
    }
    
    if (!encoder->frame_ready || !encoder->current_buffer) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "No frame encoded");
        return false;
    }
    
    *jpeg_size = encoder->current_buffer->length;
    *jpeg_data = malloc(*jpeg_size);
    if (!*jpeg_data) {
        snprintf(encoder->error_message, sizeof(encoder->error_message), 
                "Failed to allocate memory for JPEG data");
        return false;
    }
    
    memcpy(*jpeg_data, encoder->current_buffer->data, *jpeg_size);
    
    return true;
#else
    snprintf(encoder->error_message, sizeof(encoder->error_message), 
            "Hardware encoder not available on this system");
    return false;
#endif
#endif
}

void mjpeg_hw_encoder_free(uint8_t* jpeg_data) {
    if (jpeg_data) {
        free(jpeg_data);
    }
}

const char* mjpeg_hw_encoder_get_error(const mjpeg_hw_encoder_t* encoder) {
    if (!encoder) return "Invalid encoder context";
    return encoder->error_message;
}

bool mjpeg_hw_encoder_available(void) {
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    return true;
#else
    return false;
#endif
#else
    return false;
#endif
}
