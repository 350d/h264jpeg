#include "h264_to_jpeg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main() {
    printf("H.264 to JPEG Hardware Pipeline Test\n");
    printf("=====================================\n\n");
    
    // Enable debug output
    h264_to_jpeg_set_debug(true);
    
    // Create minimal test data
    uint8_t test_data[] = {
        // SPS NAL unit (4-byte length + NAL data)
        0x00, 0x00, 0x00, 0x14,  // Length: 20 bytes
        0x67, 0x42, 0x00, 0x1e,  // NAL header: SPS (type 7)
        0x80, 0x80, 0x80, 0x80,  // Mock SPS data
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        
        // PPS NAL unit
        0x00, 0x00, 0x00, 0x08,  // Length: 8 bytes
        0x68, 0x80, 0x80, 0x80,  // NAL header: PPS (type 8)
        0x80, 0x80, 0x80, 0x80,  // Mock PPS data
        
        // IDR frame NAL unit
        0x00, 0x00, 0x00, 0x20,  // Length: 32 bytes
        0x65, 0x80, 0x80, 0x80,  // NAL header: IDR (type 5)
        0x80, 0x80, 0x80, 0x80,  // Mock IDR data
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80
    };
    
    size_t data_size = sizeof(test_data);
    printf("Test data size: %zu bytes\n", data_size);
    
    uint8_t* jpeg_data = NULL;
    size_t jpeg_size = 0;
    
    printf("Attempting hardware pipeline conversion...\n");
    
    if (h264_to_jpeg(test_data, data_size, &jpeg_data, &jpeg_size, 85)) {
        printf("✓ Hardware pipeline conversion successful!\n");
        printf("JPEG size: %zu bytes\n", jpeg_size);
        
        // Save to file
        FILE* file = fopen("tmp/hw_pipeline_output.jpg", "wb");
        if (file) {
            fwrite(jpeg_data, 1, jpeg_size, file);
            fclose(file);
            printf("Saved to: tmp/hw_pipeline_output.jpg\n");
        } else {
            printf("Warning: Could not save to file\n");
        }
        
        h264_to_jpeg_free(jpeg_data);
    } else {
        printf("✗ Hardware pipeline conversion failed: %s\n", h264_to_jpeg_get_error());
    }
    
    printf("\nTest completed!\n");
    return 0;
}
