#include "h264_to_jpeg.h"
#include "h264_hw_decoder.h"
#include "mjpeg_hw_encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

static char g_error_message[256] = {0};
static bool g_debug_enabled = false;

static void debug_printf(const char* format, ...) {
    if (!g_debug_enabled) return;
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

bool h264_to_jpeg(const uint8_t* h264_data, 
                  size_t h264_size, 
                  uint8_t** jpeg_data, 
                  size_t* jpeg_size,
                  int quality) {
    if (!h264_data || h264_size == 0 || !jpeg_data || !jpeg_size) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Invalid parameters");
        return false;
    }
    
    if (quality < 1 || quality > 100) {
        quality = 85;
    }
    
    debug_printf("Starting H.264 to JPEG conversion (size: %zu, quality: %d)\n", 
                h264_size, quality);
    
    const yuv420_frame_t* yuv_frame = NULL;
    bool decode_success = false;
    
    if (!h264_hw_decoder_available()) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Hardware decoder not available on this system");
        return false;
    }
    
    debug_printf("Using hardware H.264 decoder\n");
    
    h264_hw_decoder_t hw_decoder;
    if (h264_hw_decoder_init(&hw_decoder)) {
        if (h264_hw_decoder_process(&hw_decoder, h264_data, h264_size)) {
            yuv_frame = h264_hw_decoder_get_frame(&hw_decoder);
            if (yuv_frame) {
                decode_success = true;
                debug_printf("Hardware decoding successful: %dx%d\n", 
                            yuv_frame->width, yuv_frame->height);
            }
        } else {
            snprintf(g_error_message, sizeof(g_error_message), 
                    "Hardware decoding failed: %s", 
                    h264_hw_decoder_get_error(&hw_decoder));
        }
        h264_hw_decoder_cleanup(&hw_decoder);
    } else {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Hardware decoder initialization failed: %s", 
                h264_hw_decoder_get_error(&hw_decoder));
    }
    
    if (!decode_success || !yuv_frame) {
        if (strlen(g_error_message) == 0) {
            snprintf(g_error_message, sizeof(g_error_message), 
                    "No frame available after H.264 decoding");
        }
        return false;
    }
    
    if (!mjpeg_hw_encoder_available()) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Hardware MJPEG encoder not available on this system");
        return false;
    }
    
    debug_printf("Using hardware MJPEG encoder\n");
    
    mjpeg_hw_encoder_t encoder;
    if (!mjpeg_hw_encoder_init(&encoder, quality)) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Hardware MJPEG encoder initialization failed: %s", 
                mjpeg_hw_encoder_get_error(&encoder));
        return false;
    }
    
    if (!mjpeg_hw_encoder_encode(&encoder, yuv_frame, jpeg_data, jpeg_size)) {
        snprintf(g_error_message, sizeof(g_error_message), 
                "Hardware MJPEG encoding failed: %s", 
                mjpeg_hw_encoder_get_error(&encoder));
        mjpeg_hw_encoder_cleanup(&encoder);
        return false;
    }
    
    debug_printf("Hardware MJPEG encoding successful (size: %zu bytes)\n", *jpeg_size);
    
    mjpeg_hw_encoder_cleanup(&encoder);
    
    return true;
}

void h264_to_jpeg_free(uint8_t* jpeg_data) {
    if (jpeg_data) {
        free(jpeg_data);
    }
}

const char* h264_to_jpeg_get_error(void) {
    return g_error_message;
}

void h264_to_jpeg_set_debug(bool enabled) {
    g_debug_enabled = enabled;
}
