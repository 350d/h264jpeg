#include "h264_hw_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef RASPBERRY_PI
static void output_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    h264_hw_decoder_t* decoder = (h264_hw_decoder_t*)port->userdata;
    
    if (decoder && buffer->length > 0) {
        decoder->current_buffer = buffer;
        decoder->frame_ready = true;
        vcos_semaphore_post(&decoder->output_semaphore);
    }
    
    mmal_buffer_header_release(buffer);
}

static void input_callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
    mmal_buffer_header_release(buffer);
}

static bool convert_mmal_to_yuv420(h264_hw_decoder_t* decoder, MMAL_BUFFER_HEADER_T* buffer) {
    if (!buffer || !buffer->data || buffer->length == 0) {
        return false;
    }
    
    int width = decoder->width;
    int height = decoder->height;
    int y_size = width * height;
    int uv_size = y_size / 4;
    
    if (decoder->current_frame.y_plane) {
        free(decoder->current_frame.y_plane);
    }
    
    decoder->current_frame.y_plane = malloc(y_size + 2 * uv_size);
    if (!decoder->current_frame.y_plane) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to allocate memory for YUV frame");
        return false;
    }
    
    decoder->current_frame.u_plane = decoder->current_frame.y_plane + y_size;
    decoder->current_frame.v_plane = decoder->current_frame.u_plane + uv_size;
    decoder->current_frame.width = width;
    decoder->current_frame.height = height;
    decoder->current_frame.y_size = y_size;
    decoder->current_frame.uv_size = uv_size;
    
    uint8_t* src = buffer->data;
    uint8_t* y_dst = decoder->current_frame.y_plane;
    uint8_t* u_dst = decoder->current_frame.u_plane;
    uint8_t* v_dst = decoder->current_frame.v_plane;
    
    memcpy(y_dst, src, y_size);
    src += y_size;
    
    memcpy(u_dst, src, uv_size);
    src += uv_size;
    
    memcpy(v_dst, src, uv_size);
    
    return true;
}
#endif

bool h264_hw_decoder_init(h264_hw_decoder_t* decoder) {
    if (!decoder) return false;
    
    memset(decoder, 0, sizeof(h264_hw_decoder_t));
    
#ifdef RASPBERRY_PI
    vcos_init();
    
    if (vcos_semaphore_create(&decoder->output_semaphore, "output_sem", 0) != VCOS_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to create semaphore");
        return false;
    }
    
    MMAL_STATUS_T status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_DECODER, &decoder->decoder);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to create decoder component: %s", mmal_status_to_string(status));
        return false;
    }
    
    decoder->input_port = decoder->decoder->input[0];
    decoder->output_port = decoder->decoder->output[0];
    
    MMAL_ES_FORMAT_T* input_format = decoder->input_port->format;
    input_format->type = MMAL_ES_TYPE_VIDEO;
    input_format->encoding = MMAL_ENCODING_H264;
    input_format->es->video.width = 0;
    input_format->es->video.height = 0;
    
    MMAL_ES_FORMAT_T* output_format = decoder->output_port->format;
    output_format->type = MMAL_ES_TYPE_VIDEO;
    output_format->encoding = MMAL_ENCODING_I420;
    output_format->es->video.width = 0;
    output_format->es->video.height = 0;
    
    status = mmal_port_format_commit(decoder->input_port);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to commit input format: %s", mmal_status_to_string(status));
        return false;
    }
    
    status = mmal_port_format_commit(decoder->output_port);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to commit output format: %s", mmal_status_to_string(status));
        return false;
    }
    
    decoder->output_port->userdata = (struct MMAL_PORT_USERDATA_T*)decoder;
    status = mmal_port_enable(decoder->output_port, output_callback);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to enable output port: %s", mmal_status_to_string(status));
        return false;
    }
    
    decoder->input_port->userdata = (struct MMAL_PORT_USERDATA_T*)decoder;
    status = mmal_port_enable(decoder->input_port, input_callback);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to enable input port: %s", mmal_status_to_string(status));
        return false;
    }
    
    decoder->input_pool = mmal_port_pool_create(decoder->input_port, 1, 0);
    if (!decoder->input_pool) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to create input pool");
        return false;
    }
    
    decoder->output_pool = mmal_port_pool_create(decoder->output_port, 1, 0);
    if (!decoder->output_pool) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to create output pool");
        return false;
    }
    
    status = mmal_component_enable(decoder->decoder);
    if (status != MMAL_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to enable decoder: %s", mmal_status_to_string(status));
        return false;
    }
    
    decoder->component_ready = true;
#else
    decoder->hw_available = false;
    snprintf(decoder->error_message, sizeof(decoder->error_message), 
            "Hardware decoder not available on this system");
#endif
    
    return true;
}

void h264_hw_decoder_cleanup(h264_hw_decoder_t* decoder) {
    if (!decoder) return;
    
#ifdef RASPBERRY_PI
    if (decoder->component_ready) {
        if (decoder->input_port) {
            mmal_port_disable(decoder->input_port);
        }
        if (decoder->output_port) {
            mmal_port_disable(decoder->output_port);
        }
        
        if (decoder->decoder) {
            mmal_component_disable(decoder->decoder);
        }
    }
    
    if (decoder->input_pool) {
        mmal_port_pool_destroy(decoder->input_port, decoder->input_pool);
    }
    if (decoder->output_pool) {
        mmal_port_pool_destroy(decoder->output_port, decoder->output_pool);
    }
    
    if (decoder->decoder) {
        mmal_component_destroy(decoder->decoder);
    }
    
    vcos_semaphore_delete(&decoder->output_semaphore);
#endif
    
    if (decoder->current_frame.y_plane) {
        free(decoder->current_frame.y_plane);
    }
    
    memset(decoder, 0, sizeof(h264_hw_decoder_t));
}

bool h264_hw_decoder_process(h264_hw_decoder_t* decoder, 
                            const uint8_t* h264_data, 
                            size_t h264_size) {
    if (!decoder || !h264_data || h264_size == 0) {
        if (decoder) {
            snprintf(decoder->error_message, sizeof(decoder->error_message), 
                    "Invalid parameters");
        }
        return false;
    }
    
#ifdef RASPBERRY_PI
    if (!decoder->component_ready) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Hardware decoder not ready");
        return false;
    }
    
    decoder->frame_ready = false;
    
    MMAL_BUFFER_HEADER_T* buffer = mmal_queue_get(decoder->input_pool->queue);
    if (!buffer) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "No input buffer available");
        return false;
    }
    
    memcpy(buffer->data, h264_data, h264_size);
    buffer->length = h264_size;
    buffer->offset = 0;
    buffer->flags = MMAL_BUFFER_HEADER_FLAG_FRAME_END;
    
    MMAL_STATUS_T status = mmal_port_send_buffer(decoder->input_port, buffer);
    if (status != MMAL_SUCCESS) {
        mmal_buffer_header_release(buffer);
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to send buffer: %s", mmal_status_to_string(status));
        return false;
    }
    
    if (vcos_semaphore_wait_timeout(&decoder->output_semaphore, 1000) != VCOS_SUCCESS) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Timeout waiting for decoded frame");
        return false;
    }
    
    if (!decoder->frame_ready || !decoder->current_buffer) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "No frame decoded");
        return false;
    }
    
    if (!convert_mmal_to_yuv420(decoder, decoder->current_buffer)) {
        snprintf(decoder->error_message, sizeof(decoder->error_message), 
                "Failed to convert MMAL frame to YUV420");
        return false;
    }
    
    decoder->width = 640;
    decoder->height = 480;
    
    return true;
#else
    snprintf(decoder->error_message, sizeof(decoder->error_message), 
            "Hardware decoder not available on this system");
    return false;
#endif
}

const yuv420_frame_t* h264_hw_decoder_get_frame(const h264_hw_decoder_t* decoder) {
    if (!decoder) return NULL;
    
    if (decoder->frame_ready && decoder->current_frame.y_plane) {
        return &decoder->current_frame;
    }
    
    return NULL;
}

const char* h264_hw_decoder_get_error(const h264_hw_decoder_t* decoder) {
    if (!decoder) return "Invalid decoder context";
    return decoder->error_message;
}

bool h264_hw_decoder_available(void) {
#ifdef RASPBERRY_PI
    return true;
#else
    return false;
#endif
}
