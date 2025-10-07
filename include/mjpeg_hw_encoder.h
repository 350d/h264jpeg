#ifndef MJPEG_HW_ENCODER_H
#define MJPEG_HW_ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "h264_hw_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RASPBERRY_PI
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/vcos/vcos.h"
#endif

typedef struct {
#ifdef RASPBERRY_PI
#ifndef NO_HARDWARE
    MMAL_COMPONENT_T* encoder;
    MMAL_PORT_T* input_port;
    MMAL_PORT_T* output_port;
    MMAL_POOL_T* input_pool;
    MMAL_POOL_T* output_pool;
    MMAL_BUFFER_HEADER_T* current_buffer;
    VCOS_SEMAPHORE_T output_semaphore;
    bool frame_ready;
    bool component_ready;
#else
    bool hw_available;
#endif
#else
    bool hw_available;
#endif
    
    char error_message[256];
    int quality;
} mjpeg_hw_encoder_t;

bool mjpeg_hw_encoder_init(mjpeg_hw_encoder_t* encoder, int quality);
void mjpeg_hw_encoder_cleanup(mjpeg_hw_encoder_t* encoder);
bool mjpeg_hw_encoder_encode(mjpeg_hw_encoder_t* encoder,
                            const yuv420_frame_t* yuv_frame,
                            uint8_t** jpeg_data,
                            size_t* jpeg_size);
void mjpeg_hw_encoder_free(uint8_t* jpeg_data);
const char* mjpeg_hw_encoder_get_error(const mjpeg_hw_encoder_t* encoder);
bool mjpeg_hw_encoder_available(void);

#ifdef __cplusplus
}
#endif

#endif // MJPEG_HW_ENCODER_H
