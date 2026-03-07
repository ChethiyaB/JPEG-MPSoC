#include <stdio.h>
#include "../nios2_common/datatype.h"
#include "../nios2_common/fifo_io.h"

// Dummy base addresses for PC simulation. 
// In Quartus, system.h will provide the real hardware addresses.
#ifndef FIFO_IN_BASE
#define FIFO_IN_BASE     0x00000000
#define FIFO_IN_CSR_BASE 0x00000000
#endif

#ifndef FIFO_OUT_BASE
#define FIFO_OUT_BASE     0x00000000
#define FIFO_OUT_CSR_BASE 0x00000000
#endif

extern void levelshift(INT16 *block);

int main() {
    printf("--- Nios II: Stage 2 (Level Shifting) ---\n");
    
    // Initialize both FIFOs
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    
    // Calculate total Minimum Coded Units (MCUs) to process
    int width = 24, height = 24;
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    
    INT16 Y_block[64], Cb_block[64], Cr_block[64];
    
    printf("Stage 2: Waiting for data from Stage 1...\n");
    
    int i;
    for (i = 0; i < total_mcus; i++) {
        // 1. BLOCKING READ: Wait for and pull blocks from Stage 1
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_block, 64);
        
        // 2. PROCESS: Apply -128 level shift
        levelshift(Y_block);
        levelshift(Cb_block);
        levelshift(Cr_block);
        
        // 3. BLOCKING WRITE: Push shifted blocks to Stage 3
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_block, 64);
    }
    
    printf("Stage 2: Finished processing %d MCUs.\n", total_mcus);
    return 0;
}