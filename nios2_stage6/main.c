#include <stdio.h>
#include <stdlib.h>
#include "../nios2_common/datatype.h"
#include "../nios2_common/fifo_io.h"

#ifndef FIFO_IN_BASE
#define FIFO_IN_BASE     0x00000000
#define FIFO_IN_CSR_BASE 0x00000000
#endif

int main() {
    printf("--- Nios II: Stage 6 (File Writing) ---\n");
    
    fifo_init(FIFO_IN_CSR_BASE);
    
    // In the Altera environment, HostFS allows us to write directly to the PC
    FILE *f_out = fopen("test_24x24_pipeline.jpg", "wb");
    if (!f_out) {
        printf("Stage 6 Error: Cannot create output file!\n");
        return -1;
    }
    
    printf("Stage 6: Waiting for compressed bitstream from Stage 5...\n");
    
    int words_written = 0;
    int bytes_written = 0;
    INT16 buffer[1];
    
    while (1) {
        // 1. BLOCKING READ: Read exactly 1 word from Stage 5
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, buffer, 1);
        
        UINT16 word = (UINT16)buffer[0];
        
        // Extract the two bytes from the 16-bit word (Big-Endian format)
        UINT8 high_byte = (word >> 8) & 0xFF;
        UINT8 low_byte  = word & 0xFF;
        
        // Write bytes to the file
        fputc(high_byte, f_out);
        fputc(low_byte, f_out);
        
        words_written++;
        bytes_written += 2;
        
        // 2. CHECK FOR END OF IMAGE (EOI)
        if (word == 0xFFD9) {
            printf("Stage 6: Detected EOI marker (0xFFD9). File complete!\n");
            break;
        }
    }
    
    fclose(f_out);
    printf("Stage 6: Successfully wrote %d bytes (%d words) to test_24x24_pipeline.jpg.\n", bytes_written, words_written);
    
    return 0;
}