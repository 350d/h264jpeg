#ifndef H264_HW_DECODER_H
#define H264_HW_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct yuv420_frame {
    uint8_t* y_plane;
    uint8_t* u_plane;
    uint8_t* v_plane;
    int width;
    int height;
    int y_size;
    int uv_size;
} yuv420_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/vcos/vcos.h"
#endif
#endif
typedef struct {
    yuv420_frame_t current_frame;
    char error_message[256];
    int width;
    int height;
    bool frame_ready;
    
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    MMAL_COMPONENT_T* decoder;
    MMAL_PORT_T* input_port;
    MMAL_PORT_T* output_port;
    MMAL_POOL_T* input_pool;
    MMAL_POOL_T* output_pool;
    MMAL_BUFFER_HEADER_T* current_buffer;
    VCOS_SEMAPHORE_T output_semaphore;
    bool component_ready;
#else
    bool hw_available;
#endif
#else
    bool hw_available;
#endif
} h264_hw_decoder_t;

bool h264_hw_decoder_init(h264_hw_decoder_t* decoder);
void h264_hw_decoder_cleanup(h264_hw_decoder_t* decoder);
bool h264_hw_decoder_process(h264_hw_decoder_t* decoder, 
                            const uint8_t* h264_data, 
                            size_t h264_size);
const yuv420_frame_t* h264_hw_decoder_get_frame(const h264_hw_decoder_t* decoder);
const char* h264_hw_decoder_get_error(const h264_hw_decoder_t* decoder);
bool h264_hw_decoder_available(void);

#ifdef __cplusplus
}
#endif

#endif // H264_HW_DECODER_H
