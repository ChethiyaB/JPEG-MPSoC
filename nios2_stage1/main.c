#include <stdio.h>
#include <stdlib.h>
#include "encoder.h"
#include "../nios2_common/fifo_io.h"

int main() {
    printf("--- Nios II: Stage 1 (RGB to YCbCr) ---\n");
    
    // In Altera Eclipse, you'd include "system.h" here to get real FIFO_OUT_BASE
    fifo_init(FIFO_OUT_CSR_BASE);
    
    int width = 24, height = 24;
    int raw_size = width * height * 3;
    UINT8 *rgb_buffer = (UINT8*)malloc(raw_size);
    
    // HostFS allows Nios II to read files from the connected PC during debug
    FILE *f_in = fopen("test_24x24.raw", "rb");
    if (!f_in) {
        printf("Stage 1 Error: Cannot find test_24x24.raw\n");
        return -1;
    }
    fread(rgb_buffer, 1, raw_size, f_in);
    fclose(f_in);
    
    printf("Stage 1: Pumping image data into FIFO...\n");
    encode_image_stage1(rgb_buffer, width, height);
    
    printf("Stage 1: Finished pumping data.\n");
    free(rgb_buffer);
    return 0;
}