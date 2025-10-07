#include "h264_to_jpeg.h"
#include "h264_hw_decoder.h"
#include "mjpeg_hw_encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Force NO_HARDWARE mode
#ifdef RASPBERRY_PI
#undef RASPBERRY_PI
#endif
#define NO_HARDWARE

int main() {
    printf("=== Safe Test (NO_HARDWARE mode) ===\n");
    
    // Test 1: Check hardware availability (should be false)
    printf("1. Checking hardware availability...\n");
    printf("   H.264 decoder available: %s\n", 
           h264_hw_decoder_available() ? "YES" : "NO");
    printf("   MJPEG encoder available: %s\n", 
           mjpeg_hw_encoder_available() ? "YES" : "NO");
    
    // Test 2: Try to initialize decoder
    printf("\n2. Testing decoder initialization...\n");
    h264_hw_decoder_t decoder;
    memset(&decoder, 0, sizeof(decoder));
    
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
        printf("   Error message: '%s'\n", h264_hw_decoder_get_error(&decoder));
    }
    
    // Test 3: Try to initialize encoder
    printf("\n3. Testing encoder initialization...\n");
    mjpeg_hw_encoder_t encoder;
    memset(&encoder, 0, sizeof(encoder));
    
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
        printf("   Error message: '%s'\n", mjpeg_hw_encoder_get_error(&encoder));
    }
    
    // Test 4: Check system info
    printf("\n4. System information...\n");
    printf("   Raspberry Pi detected: %s\n", 
#ifdef RASPBERRY_PI
           "YES"
#else
           "NO"
#endif
    );
    printf("   NO_HARDWARE flag: %s\n", 
#ifdef NO_HARDWARE
           "YES"
#else
           "NO"
#endif
    );
    
    printf("\n=== Safe test completed successfully ===\n");
    return 0;
}
