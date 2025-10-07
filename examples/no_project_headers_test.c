#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Simulate the project structures without including headers
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

int main() {
    printf("=== No Project Headers Test ===\n");
    
    // Test 1: Basic operations
    printf("1. Basic printf works\n");
    
    // Test 2: Memory operations
    char* test = malloc(100);
    if (test) {
        strcpy(test, "No Project Headers Test");
        printf("2. Memory allocation works: %s\n", test);
        free(test);
        printf("3. Memory deallocation works\n");
    }
    
    // Test 3: Structure operations (simulating project structures)
    printf("4. Testing structure operations...\n");
    h264_hw_decoder_t decoder;
    memset(&decoder, 0, sizeof(decoder));
    decoder.hw_available = false;
    decoder.width = 640;
    decoder.height = 480;
    
    mjpeg_hw_encoder_t encoder;
    memset(&encoder, 0, sizeof(encoder));
    encoder.hw_available = false;
    encoder.width = 640;
    encoder.height = 480;
    encoder.quality = 85;
    
    printf("   Decoder hw_available: %s\n", decoder.hw_available ? "YES" : "NO");
    printf("   Encoder hw_available: %s\n", encoder.hw_available ? "YES" : "NO");
    
    // Test 4: Integer operations
    int test_int = 314159;
    printf("5. Integer operations: %d\n", test_int);
    
    printf("=== No project headers test completed successfully ===\n");
    return 0;
}
