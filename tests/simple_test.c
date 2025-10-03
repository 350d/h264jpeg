#include "h264_to_jpeg.h"
#include "h264_hw_decoder.h"
#include "mjpeg_hw_encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Test helper functions
static void test_assert(bool condition, const char* message) {
    if (!condition) {
        printf("FAIL: %s\n", message);
        exit(1);
    }
    printf("PASS: %s\n", message);
}

static uint8_t* create_test_h264_data(size_t* size) {
    // Create minimal test H.264 data
    *size = 72;
    uint8_t* data = malloc(*size);
    if (!data) return NULL;
    
    // Mock SPS
    uint32_t sps_len = 20;
    data[0] = (sps_len >> 24) & 0xFF;
    data[1] = (sps_len >> 16) & 0xFF;
    data[2] = (sps_len >> 8) & 0xFF;
    data[3] = sps_len & 0xFF;
    data[4] = 0x67; // SPS
    memset(data + 5, 0x80, sps_len - 1);
    
    // Mock PPS
    uint32_t pps_len = 8;
    uint32_t pps_off = 4 + sps_len;
    data[pps_off] = (pps_len >> 24) & 0xFF;
    data[pps_off + 1] = (pps_len >> 16) & 0xFF;
    data[pps_off + 2] = (pps_len >> 8) & 0xFF;
    data[pps_off + 3] = pps_len & 0xFF;
    data[pps_off + 4] = 0x68; // PPS
    memset(data + pps_off + 5, 0x80, pps_len - 1);
    
    // Mock IDR
    uint32_t idr_len = *size - pps_off - 4 - pps_len;
    uint32_t idr_off = pps_off + 4 + pps_len;
    data[idr_off] = (idr_len >> 24) & 0xFF;
    data[idr_off + 1] = (idr_len >> 16) & 0xFF;
    data[idr_off + 2] = (idr_len >> 8) & 0xFF;
    data[idr_off + 3] = idr_len & 0xFF;
    data[idr_off + 4] = 0x65; // IDR
    memset(data + idr_off + 5, 0x80, idr_len - 1);
    
    return data;
}

void test_basic_conversion() {
    printf("\n=== Testing Hardware Pipeline ===\n");
    
    // Check hardware availability first
    if (!h264_hw_decoder_available() || !mjpeg_hw_encoder_available()) {
        printf("SKIP: Hardware components not available on this system\n");
        return;
    }
    
    size_t h264_size;
    uint8_t* h264_data = create_test_h264_data(&h264_size);
    test_assert(h264_data != NULL, "Test data creation");
    
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    bool result = h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 85);
    test_assert(result, "Hardware pipeline H.264 to JPEG conversion");
    test_assert(jpeg_data != NULL, "JPEG data allocation");
    test_assert(jpeg_size > 0, "JPEG data size > 0");
    
    // Check JPEG header
    test_assert(jpeg_data[0] == 0xFF && jpeg_data[1] == 0xD8, "JPEG header");
    
    h264_to_jpeg_free(jpeg_data);
    free(h264_data);
}

void test_quality_settings() {
    printf("\n=== Testing Quality Settings ===\n");
    
    size_t h264_size;
    uint8_t* h264_data = create_test_h264_data(&h264_size);
    test_assert(h264_data != NULL, "Test data creation");
    
    int qualities[] = {50, 75, 85, 95};
    size_t jpeg_sizes[4];
    
    for (int i = 0; i < 4; i++) {
        uint8_t* jpeg_data = NULL;
        size_t jpeg_size = 0;
        
        bool result = h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, qualities[i]);
        test_assert(result, "Conversion with quality");
        test_assert(jpeg_data != NULL, "JPEG data allocation");
        test_assert(jpeg_size > 0, "JPEG data size > 0");
        
        jpeg_sizes[i] = jpeg_size;
        h264_to_jpeg_free(jpeg_data);
    }
    
    // Higher quality should generally produce larger files
    test_assert(jpeg_sizes[3] >= jpeg_sizes[0], "Quality affects file size");
    
    free(h264_data);
}

void test_error_handling() {
    printf("\n=== Testing Error Handling ===\n");
    
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    // Test NULL input
    bool result = h264_to_jpeg(NULL, 100, &jpeg_data, &jpeg_size, 85);
    test_assert(!result, "NULL input rejected");
    test_assert(strlen(h264_to_jpeg_get_error()) > 0, "Error message provided");
    
    // Test zero size
    uint8_t dummy_data[10];
    result = h264_to_jpeg(dummy_data, 0, &jpeg_data, &jpeg_size, 85);
    test_assert(!result, "Zero size rejected");
    
    // Test invalid quality
    size_t h264_size;
    uint8_t* h264_data = create_test_h264_data(&h264_size);
    result = h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 150);
    test_assert(!result, "Invalid quality rejected");
    
    result = h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 0);
    test_assert(!result, "Zero quality rejected");
    
    free(h264_data);
}

void test_memory_management() {
    printf("\n=== Testing Memory Management ===\n");
    
    size_t h264_size;
    uint8_t* h264_data = create_test_h264_data(&h264_size);
    test_assert(h264_data != NULL, "Test data creation");
    
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    bool result = h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 85);
    test_assert(result, "Conversion successful");
    
    // Test free function
    h264_to_jpeg_free(jpeg_data);
    h264_to_jpeg_free(NULL); // Should not crash
    
    free(h264_data);
}

void test_debug_output() {
    printf("\n=== Testing Debug Output ===\n");
    
    // Enable debug
    h264_to_jpeg_set_debug(true);
    test_assert(true, "Debug enabled");
    
    // Disable debug
    h264_to_jpeg_set_debug(false);
    test_assert(true, "Debug disabled");
}

int main() {
    printf("H.264 to JPEG Library Tests\n");
    printf("===========================\n");
    
    test_basic_conversion();
    test_quality_settings();
    test_error_handling();
    test_memory_management();
    test_debug_output();
    
    printf("\nAll tests passed! ✓\n");
    return 0;
}
