#include <stdio.h>
#include <stdlib.h>
#include "datatype.h"
#include "encoder_sw.h"

int main() {
    int width = 24, height = 24; // Default dimensions for test_24x24.raw
    
    printf("--- Phase 0: Software JPEG Encoder ---\n");
    
    // Allocate memory
    int raw_size = width * height * 3;
    UINT8 *rgb_buffer = (UINT8*)malloc(raw_size);
    UINT8 *jpeg_buffer = (UINT8*)malloc(1024 * 1024); // 1 MB buffer to prevent overflow
    
    if (!rgb_buffer || !jpeg_buffer) {
        printf("Memory allocation failed!\n");
        return -1;
    }
    
    // Read RAW image
    FILE *f_in = fopen("test_vectors/test_24x24.raw", "rb");
    if (!f_in) {
        printf("Failed to open test_24x24.raw! Please ensure it is in the same directory.\n");
        return -1;
    }
    fread(rgb_buffer, 1, raw_size, f_in);
    fclose(f_in);
    
    // Encode
    printf("Encoding %dx%d image...\n", width, height);
    int jpeg_size = encode_image_sw(rgb_buffer, jpeg_buffer, width, height);
    
    // Write Output
    FILE *f_out = fopen("test_vectors/test_24x24_golden.jpg", "wb");
    if (!f_out) {
        printf("Failed to create output file!\n");
        return -1;
    }
    fwrite(jpeg_buffer, 1, jpeg_size, f_out);
    fclose(f_out);
    
    printf("Success! Wrote %d bytes to test_24x24_golden.jpg\n", jpeg_size);
    
    free(rgb_buffer);
    free(jpeg_buffer);
    return 0;
}