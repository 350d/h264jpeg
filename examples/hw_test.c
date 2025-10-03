#include "h264_to_jpeg.h"
#include "h264_hw_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Generate mock H.264 data for testing
static uint8_t* generate_mock_h264_data(size_t* size) {
    *size = 1024;
    uint8_t* data = malloc(*size);
    if (!data) return NULL;
    
    // Mock SPS NAL unit (4-byte length + NAL data)
    uint32_t sps_length = 20;
    data[0] = (sps_length >> 24) & 0xFF;
    data[1] = (sps_length >> 16) & 0xFF;
    data[2] = (sps_length >> 8) & 0xFF;
    data[3] = sps_length & 0xFF;
    data[4] = 0x67; // NAL header: SPS (type 7)
    memset(data + 5, 0x80, sps_length - 1);
    
    // Mock PPS NAL unit
    uint32_t pps_length = 8;
    uint32_t pps_offset = 4 + sps_length;
    data[pps_offset] = (pps_length >> 24) & 0xFF;
    data[pps_offset + 1] = (pps_length >> 16) & 0xFF;
    data[pps_offset + 2] = (pps_length >> 8) & 0xFF;
    data[pps_offset + 3] = pps_length & 0xFF;
    data[pps_offset + 4] = 0x68; // NAL header: PPS (type 8)
    memset(data + pps_offset + 5, 0x80, pps_length - 1);
    
    // Mock IDR frame NAL unit
    uint32_t idr_length = *size - pps_offset - 4 - pps_length;
    uint32_t idr_offset = pps_offset + 4 + pps_length;
    data[idr_offset] = (idr_length >> 24) & 0xFF;
    data[idr_offset + 1] = (idr_length >> 16) & 0xFF;
    data[idr_offset + 2] = (idr_length >> 8) & 0xFF;
    data[idr_offset + 3] = idr_length & 0xFF;
    data[idr_offset + 4] = 0x65; // NAL header: IDR (type 5)
    memset(data + idr_offset + 5, 0x80, idr_length - 1);
    
    return data;
}

int main(int argc, char* argv[]) {
    printf("H.264 Hardware Decoder Test\n");
    printf("===========================\n\n");
    
    // Enable debug output
    h264_to_jpeg_set_debug(true);
    
    // Check hardware availability
    printf("Hardware decoder available: %s\n", 
           h264_hw_decoder_available() ? "YES" : "NO");
    
    // Generate mock H.264 data
    size_t h264_size;
    uint8_t* h264_data = generate_mock_h264_data(&h264_size);
    if (!h264_data) {
        printf("Error: Failed to generate mock H.264 data\n");
        return 1;
    }
    
    printf("Generated mock H.264 data (%zu bytes)\n", h264_size);
    
    // Test conversion
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    printf("\nAttempting H.264 to JPEG conversion...\n");
    
    if (h264_to_jpeg(h264_data, h264_size, &jpeg_data, &jpeg_size, 85)) {
        printf("✓ Conversion successful!\n");
        printf("JPEG size: %zu bytes\n", jpeg_size);
        
        // Save to file
        char filename[64];
        snprintf(filename, sizeof(filename), "tmp/hw_test_output.jpg");
        
        FILE* file = fopen(filename, "wb");
        if (file) {
            fwrite(jpeg_data, 1, jpeg_size, file);
            fclose(file);
            printf("Saved to: %s\n", filename);
        } else {
            printf("Warning: Could not save to file\n");
        }
        
        h264_to_jpeg_free(jpeg_data);
    } else {
        printf("✗ Conversion failed: %s\n", h264_to_jpeg_get_error());
    }
    
    free(h264_data);
    
    printf("\nTest completed!\n");
    return 0;
}
