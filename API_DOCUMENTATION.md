# API Documentation

## Overview

This document provides comprehensive API documentation for the H.264 to JPEG hardware pipeline library. All comments have been moved from the source code to this documentation for better maintainability.

## Core API

### h264_to_jpeg.h

Main API interface for H.264 to JPEG conversion.

#### Functions

##### `bool h264_to_jpeg(const uint8_t* h264_data, size_t h264_size, uint8_t** jpeg_data, size_t* jpeg_size, int quality)`

Converts H.264 I-frame data to JPEG image using hardware acceleration.

**Parameters:**
- `h264_data`: Raw H.264 data with 4-byte length prefix NAL units
- `h264_size`: Size of H.264 data in bytes
- `jpeg_data`: Output buffer for JPEG data (allocated by function)
- `jpeg_size`: Output size of JPEG data in bytes
- `quality`: JPEG quality (1-100, default 85)

**Returns:**
- `true` on success, `false` on error

**Description:**
This is the main function that orchestrates the entire H.264 to JPEG conversion pipeline. It uses hardware acceleration on Raspberry Pi for both H.264 decoding and JPEG encoding.

##### `void h264_to_jpeg_free(uint8_t* jpeg_data)`

Frees memory allocated by h264_to_jpeg.

**Parameters:**
- `jpeg_data`: JPEG data buffer to free

**Description:**
Must be called to free memory allocated by h264_to_jpeg to prevent memory leaks.

##### `const char* h264_to_jpeg_get_error(void)`

Gets the last error message.

**Returns:**
- Error message string (NULL if no error)

**Description:**
Returns a human-readable error message describing the last error that occurred.

##### `void h264_to_jpeg_set_debug(bool enabled)`

Enables or disables debug output.

**Parameters:**
- `enabled`: Enable/disable debug output

**Description:**
When enabled, provides detailed logging of the conversion process for debugging purposes.

## Hardware H.264 Decoder

### h264_hw_decoder.h

Hardware H.264 decoder using VideoCore IV GPU on Raspberry Pi.

#### Data Structures

##### `yuv420_frame_t`

Structure representing a YUV420 frame.

**Fields:**
- `uint8_t* y_plane`: Y (luminance) plane data
- `uint8_t* u_plane`: U (chroma) plane data
- `uint8_t* v_plane`: V (chroma) plane data
- `int width`: Frame width in pixels
- `int height`: Frame height in pixels
- `int y_size`: Size of Y plane in bytes
- `int uv_size`: Size of U/V plane in bytes

**Description:**
YUV420 is a color space format where the Y (luminance) plane is full resolution and the U and V (chroma) planes are quarter resolution. This format is commonly used in video processing.

##### `h264_hw_decoder_t`

Hardware H.264 decoder context.

**Fields:**
- `yuv420_frame_t current_frame`: Current decoded frame
- `char error_message[256]`: Last error message
- `int width`: Frame width
- `int height`: Frame height
- `bool frame_ready`: Frame ready flag

**Raspberry Pi specific fields:**
- `MMAL_COMPONENT_T* decoder`: MMAL decoder component
- `MMAL_PORT_T* input_port`: Input port for H.264 data
- `MMAL_PORT_T* output_port`: Output port for YUV420 data
- `MMAL_POOL_T* input_pool`: Input buffer pool
- `MMAL_POOL_T* output_pool`: Output buffer pool
- `MMAL_BUFFER_HEADER_T* current_buffer`: Current buffer
- `VCOS_SEMAPHORE_T output_semaphore`: Output synchronization
- `bool component_ready`: Component ready flag

**Non-Pi systems:**
- `bool hw_available`: Hardware availability flag

#### Functions

##### `bool h264_hw_decoder_init(h264_hw_decoder_t* decoder)`

Initializes hardware H.264 decoder.

**Parameters:**
- `decoder`: Decoder context to initialize

**Returns:**
- `true` on success, `false` on error

**Description:**
Initializes the MMAL decoder component and sets up input/output ports. On non-Pi systems, sets hw_available to false.

##### `void h264_hw_decoder_cleanup(h264_hw_decoder_t* decoder)`

Cleans up hardware H.264 decoder.

**Parameters:**
- `decoder`: Decoder context to cleanup

**Description:**
Releases all MMAL resources and frees allocated memory. Safe to call multiple times.

##### `bool h264_hw_decoder_process(h264_hw_decoder_t* decoder, const uint8_t* h264_data, size_t h264_size)`

Processes H.264 data using hardware decoder.

**Parameters:**
- `decoder`: Decoder context
- `h264_data`: Raw H.264 data with 4-byte length prefix
- `h264_size`: Size of H.264 data

**Returns:**
- `true` if I-frame was decoded, `false` otherwise

**Description:**
Sends H.264 data to the hardware decoder and waits for completion. Only processes I-frames (keyframes).

##### `const yuv420_frame_t* h264_hw_decoder_get_frame(const h264_hw_decoder_t* decoder)`

Gets decoded YUV420 frame.

**Parameters:**
- `decoder`: Decoder context

**Returns:**
- Pointer to YUV420 frame (NULL if not ready)

**Description:**
Returns the last decoded frame. The frame data is valid until the next call to process or cleanup.

##### `const char* h264_hw_decoder_get_error(const h264_hw_decoder_t* decoder)`

Gets last error message.

**Parameters:**
- `decoder`: Decoder context

**Returns:**
- Error message string

**Description:**
Returns a human-readable error message describing the last error that occurred.

##### `bool h264_hw_decoder_available(void)`

Checks if hardware decoder is available.

**Returns:**
- `true` if hardware decoder is available, `false` otherwise

**Description:**
Returns true only on Raspberry Pi systems with VideoCore IV GPU support.

## Hardware MJPEG Encoder

### mjpeg_hw_encoder.h

Hardware MJPEG encoder using VideoCore IV GPU on Raspberry Pi.

#### Data Structures

##### `mjpeg_hw_encoder_t`

Hardware MJPEG encoder context.

**Fields:**
- `char error_message[256]`: Last error message
- `int quality`: JPEG quality setting

**Raspberry Pi specific fields:**
- `MMAL_COMPONENT_T* encoder`: MMAL encoder component
- `MMAL_PORT_T* input_port`: Input port for YUV420 data
- `MMAL_PORT_T* output_port`: Output port for JPEG data
- `MMAL_POOL_T* input_pool`: Input buffer pool
- `MMAL_POOL_T* output_pool`: Output buffer pool
- `MMAL_BUFFER_HEADER_T* current_buffer`: Current buffer
- `VCOS_SEMAPHORE_T output_semaphore`: Output synchronization
- `bool frame_ready`: Frame ready flag
- `bool component_ready`: Component ready flag

**Non-Pi systems:**
- `bool hw_available`: Hardware availability flag

#### Functions

##### `bool mjpeg_hw_encoder_init(mjpeg_hw_encoder_t* encoder, int quality)`

Initializes hardware MJPEG encoder.

**Parameters:**
- `encoder`: Encoder context to initialize
- `quality`: JPEG quality (1-100)

**Returns:**
- `true` on success, `false` on error

**Description:**
Initializes the MMAL encoder component and sets up input/output ports. Quality must be between 1 and 100.

##### `void mjpeg_hw_encoder_cleanup(mjpeg_hw_encoder_t* encoder)`

Cleans up hardware MJPEG encoder.

**Parameters:**
- `encoder`: Encoder context to cleanup

**Description:**
Releases all MMAL resources and frees allocated memory. Safe to call multiple times.

##### `bool mjpeg_hw_encoder_encode(mjpeg_hw_encoder_t* encoder, const yuv420_frame_t* yuv_frame, uint8_t** jpeg_data, size_t* jpeg_size)`

Encodes YUV420 frame to JPEG using hardware.

**Parameters:**
- `encoder`: Encoder context
- `yuv_frame`: YUV420 frame to encode
- `jpeg_data`: Output buffer for JPEG data (allocated by function)
- `jpeg_size`: Output size of JPEG data

**Returns:**
- `true` on success, `false` on error

**Description:**
Converts YUV420 frame to JPEG format using hardware acceleration. The jpeg_data buffer is allocated by the function and must be freed using mjpeg_hw_encoder_free.

##### `void mjpeg_hw_encoder_free(uint8_t* jpeg_data)`

Frees JPEG data allocated by encoder.

**Parameters:**
- `jpeg_data`: JPEG data to free

**Description:**
Frees memory allocated by mjpeg_hw_encoder_encode.

##### `const char* mjpeg_hw_encoder_get_error(const mjpeg_hw_encoder_t* encoder)`

Gets last error message.

**Parameters:**
- `encoder`: Encoder context

**Returns:**
- Error message string

**Description:**
Returns a human-readable error message describing the last error that occurred.

##### `bool mjpeg_hw_encoder_available(void)`

Checks if hardware encoder is available.

**Returns:**
- `true` if hardware encoder is available, `false` otherwise

**Description:**
Returns true only on Raspberry Pi systems with VideoCore IV GPU support.

## V4L2 Test Utility

### v4l2_h264_test.c

Real-world test utility for V4L2 camera integration.

#### Functions

##### `static void signal_handler(int sig)`

Signal handler for graceful shutdown.

**Parameters:**
- `sig`: Signal number

**Description:**
Handles SIGINT and SIGTERM signals to gracefully shut down the capture loop.

##### `static bool v4l2_set_format(int fd, int width, int height)`

Sets V4L2 format to H.264.

**Parameters:**
- `fd`: V4L2 device file descriptor
- `width`: Video width in pixels
- `height`: Video height in pixels

**Returns:**
- `true` on success, `false` on error

**Description:**
Configures the V4L2 device to output H.264 format at the specified resolution.

##### `static bool v4l2_request_buffers(int fd, int count)`

Requests V4L2 buffers.

**Parameters:**
- `fd`: V4L2 device file descriptor
- `count`: Number of buffers to request

**Returns:**
- `true` on success, `false` on error

**Description:**
Requests memory-mapped buffers from the V4L2 device for video capture.

##### `static bool v4l2_map_buffers(int fd)`

Maps V4L2 buffers to memory.

**Parameters:**
- `fd`: V4L2 device file descriptor

**Returns:**
- `true` on success, `false` on error

**Description:**
Maps the requested V4L2 buffers to user space memory for data access.

##### `static bool v4l2_start_capture(int fd)`

Starts V4L2 capture.

**Parameters:**
- `fd`: V4L2 device file descriptor

**Returns:**
- `true` on success, `false` on error

**Description:**
Starts the video capture stream on the V4L2 device.

##### `static bool v4l2_stop_capture(int fd)`

Stops V4L2 capture.

**Parameters:**
- `fd`: V4L2 device file descriptor

**Returns:**
- `true` on success, `false` on error

**Description:**
Stops the video capture stream on the V4L2 device.

##### `static bool is_idr_frame(const uint8_t* data, size_t size)`

Detects IDR frames in H.264 data.

**Parameters:**
- `data`: H.264 data buffer
- `size`: Size of data buffer

**Returns:**
- `true` if IDR frame detected, `false` otherwise

**Description:**
Parses H.264 NAL units to detect IDR (Instantaneous Decoder Refresh) frames, which are keyframes suitable for JPEG conversion.

##### `static bool process_h264_frame(const uint8_t* h264_data, size_t h264_size, int frame_number)`

Processes H.264 frame and converts to JPEG.

**Parameters:**
- `h264_data`: H.264 frame data
- `h264_size`: Size of frame data
- `frame_number`: Frame number for logging

**Returns:**
- `true` on success, `false` on error

**Description:**
Checks if the frame is an IDR frame and converts it to JPEG using the hardware pipeline. Saves JPEG files to the tmp/ directory.

##### `static void capture_loop(int fd)`

Main capture loop.

**Parameters:**
- `fd`: V4L2 device file descriptor

**Description:**
Main loop that continuously captures frames from the V4L2 device, processes them, and converts IDR frames to JPEG. Prints statistics every 30 frames.

##### `static void cleanup(int fd)`

Cleanup function.

**Parameters:**
- `fd`: V4L2 device file descriptor

**Description:**
Cleans up all resources including stopping capture, closing device, and unmapping buffers.

##### `int main(int argc, char* argv[])`

Main function.

**Parameters:**
- `argc`: Argument count
- `argv`: Argument vector

**Returns:**
- 0 on success, 1 on error

**Description:**
Main function that initializes the V4L2 test, sets up signal handlers, opens the video device, and runs the capture loop.

**Command line arguments:**
- `argv[1]`: V4L2 device path (default: /dev/video0)
- `argv[2]`: Video width (default: 1280)
- `argv[3]`: Video height (default: 720)

## Error Handling

All functions return boolean values or error codes to indicate success or failure. Error messages can be retrieved using the appropriate `get_error()` function for each component.

### Common Error Conditions

1. **Hardware not available**: Returned when running on non-Pi systems
2. **Invalid parameters**: Returned when NULL pointers or invalid values are passed
3. **Memory allocation failure**: Returned when malloc() fails
4. **MMAL initialization failure**: Returned when MMAL components fail to initialize
5. **V4L2 device error**: Returned when V4L2 operations fail
6. **Timeout**: Returned when operations exceed timeout limits

## Memory Management

The library handles memory allocation and deallocation automatically. Users must call the appropriate `free()` functions to prevent memory leaks:

- `h264_to_jpeg_free()` for JPEG data
- `mjpeg_hw_encoder_free()` for JPEG data from encoder
- `h264_hw_decoder_cleanup()` for decoder cleanup
- `mjpeg_hw_encoder_cleanup()` for encoder cleanup

## Thread Safety

The library is not thread-safe. Each thread should use its own decoder and encoder contexts.

## Platform Support

- **Raspberry Pi**: Full hardware acceleration support
- **Other systems**: Graceful fallback with error messages

## Performance Considerations

- Hardware acceleration provides 10-20x performance improvement
- Memory usage is optimized for embedded systems
- GPU memory split should be at least 128MB on Raspberry Pi
- V4L2 buffer count affects memory usage and performance
