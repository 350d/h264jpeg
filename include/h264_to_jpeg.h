#ifndef H264_TO_JPEG_H
#define H264_TO_JPEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

bool h264_to_jpeg(const uint8_t* h264_data, 
                  size_t h264_size, 
                  uint8_t** jpeg_data, 
                  size_t* jpeg_size,
                  int quality);
void h264_to_jpeg_free(uint8_t* jpeg_data);
const char* h264_to_jpeg_get_error(void);
void h264_to_jpeg_set_debug(bool enabled);

#ifdef __cplusplus
}
#endif

#endif // H264_TO_JPEG_H
