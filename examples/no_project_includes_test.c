#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Define project structures without including any project headers
typedef struct {
    uint8_t* y_plane;
    uint8_t* u_plane;
    uint8_t* v_plane;
    int width;
    int height;
    int y_size;
    int uv_size;
} yuv420_frame_t;

typedef struct {
    yuv420_frame_t current_frame;
    char error_message[256];
    int width;
    int height;
    bool frame_ready;
    bool hw_available;
} h264_hw_decoder_t;

typedef struct {
    char error_message[256];
    int width;
    int height;
    int quality;
    bool hw_available;
} mjpeg_hw_encoder_t;

// Simulate project functions without including headers
bool h264_hw_decoder_available(void) {
    return false; // Always return false for NO_HARDWARE mode
}

bool mjpeg_hw_encoder_available(void) {
    return false; // Always return false for NO_HARDWARE mode
}

bool h264_hw_decoder_init(h264_hw_decoder_t* decoder) {
    if (!decoder) return false;
    
    memset(decoder, 0, sizeof(h264_hw_decoder_t));
    decoder->hw_available = false;
    strcpy(decoder->error_message, "Hardware not available (NO_HARDWARE mode)");
    return true;
}

void h264_hw_decoder_cleanup(h264_hw_decoder_t* decoder) {
    if (!decoder) return;
    
    if (decoder->current_frame.y_plane) {
        free(decoder->current_frame.y_plane);
        decoder->current_frame.y_plane = NULL;
    }
    
    memset(decoder, 0, sizeof(h264_hw_decoder_t));
}

const char* h264_hw_decoder_get_error(const h264_hw_decoder_t* decoder) {
    return decoder ? decoder->error_message : "Invalid decoder";
}

bool mjpeg_hw_encoder_init(mjpeg_hw_encoder_t* encoder, int quality) {
    if (!encoder) return false;
    
    memset(encoder, 0, sizeof(mjpeg_hw_encoder_t));
    encoder->hw_available = false;
    encoder->quality = quality;
    strcpy(encoder->error_message, "Hardware not available (NO_HARDWARE mode)");
    return true;
}

void mjpeg_hw_encoder_cleanup(mjpeg_hw_encoder_t* encoder) {
    if (!encoder) return;
    memset(encoder, 0, sizeof(mjpeg_hw_encoder_t));
}

const char* mjpeg_hw_encoder_get_error(const mjpeg_hw_encoder_t* encoder) {
    return encoder ? encoder->error_message : "Invalid encoder";
}

// Simulate h264_to_jpeg function without including headers
bool h264_to_jpeg(const uint8_t* h264_data, size_t h264_size, 
                  uint8_t** jpeg_data, size_t* jpeg_size, int quality) {
    if (!h264_data || !jpeg_data || !jpeg_size) {
        return false;
    }
    
    // Simulate conversion failure in NO_HARDWARE mode
    *jpeg_data = NULL;
    *jpeg_size = 0;
    return false;
}

const char* h264_to_jpeg_get_error(void) {
    return "Hardware not available (NO_HARDWARE mode)";
}

void h264_to_jpeg_free(uint8_t* jpeg_data) {
    if (jpeg_data) {
        free(jpeg_data);
    }
}

int main() {
    printf("=== No Project Includes Test ===\n");
    
    // Test 1: Check hardware availability (should be false)
    printf("1. Checking hardware availability...\n");
    printf("   H.264 decoder available: %s\n", 
           h264_hw_decoder_available() ? "YES" : "NO");
    printf("   MJPEG encoder available: %s\n", 
           mjpeg_hw_encoder_available() ? "YES" : "NO");
    
    // Test 2: Try to initialize decoder
    printf("\n2. Testing decoder initialization...\n");
    h264_hw_decoder_t decoder;
    
    printf("   Initializing decoder...\n");
    if (h264_hw_decoder_init(&decoder)) {
        printf("   ✓ Decoder init successful\n");
        printf("   Hardware available: %s\n", decoder.hw_available ? "YES" : "NO");
        printf("   Error message: '%s'\n", h264_hw_decoder_get_error(&decoder));
        
        printf("   Cleaning up decoder...\n");
        h264_hw_decoder_cleanup(&decoder);
        printf("   ✓ Decoder cleanup successful\n");
    } else {
        printf("   ✗ Decoder init failed\n");
    }
    
    // Test 3: Try to initialize encoder
    printf("\n3. Testing encoder initialization...\n");
    mjpeg_hw_encoder_t encoder;
    
    printf("   Initializing encoder...\n");
    if (mjpeg_hw_encoder_init(&encoder, 85)) {
        printf("   ✓ Encoder init successful\n");
        printf("   Hardware available: %s\n", encoder.hw_available ? "YES" : "NO");
        printf("   Error message: '%s'\n", mjpeg_hw_encoder_get_error(&encoder));
        
        printf("   Cleaning up encoder...\n");
        mjpeg_hw_encoder_cleanup(&encoder);
        printf("   ✓ Encoder cleanup successful\n");
    } else {
        printf("   ✗ Encoder init failed\n");
    }
    
    // Test 4: Try h264_to_jpeg function
    printf("\n4. Testing h264_to_jpeg function...\n");
    uint8_t test_data[] = {0x00, 0x00, 0x00, 0x01, 0x67}; // Fake H.264 data
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    if (h264_to_jpeg(test_data, sizeof(test_data), &jpeg_data, &jpeg_size, 85)) {
        printf("   ✓ Conversion successful\n");
        h264_to_jpeg_free(jpeg_data);
    } else {
        printf("   ✗ Conversion failed (expected): %s\n", h264_to_jpeg_get_error());
    }
    
    // Test 5: Integer operations
    int test_int = 314159;
    printf("\n5. Integer operations: %d\n", test_int);
    
    printf("\n=== No project includes test completed successfully ===\n");
    return 0;
}
