#include <stdio.h>
#include "../nios2_common/datatype.h"
#include "../nios2_common/fifo_io.h"

// Dummy base addresses for PC simulation
#ifndef FIFO_IN_BASE
#define FIFO_IN_BASE     0x00000000
#define FIFO_IN_CSR_BASE 0x00000000
#endif

#ifndef FIFO_OUT_BASE
#define FIFO_OUT_BASE     0x00000000
#define FIFO_OUT_CSR_BASE 0x00000000
#endif

extern void dct(INT16 *block);

int main() {
    printf("--- Nios II: Stage 3 (Discrete Cosine Transform) ---\n");
    
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    
    int width = 24, height = 24;
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    
    INT16 Y_block[64], Cb_block[64], Cr_block[64];
    
    printf("Stage 3: Waiting for data from Stage 2...\n");
    
    int i;
    for (i = 0; i < total_mcus; i++) {
        // 1. BLOCKING READ: Pull level-shifted blocks from Stage 2
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_block, 64);
        
        // 2. PROCESS: Apply 2D DCT
        dct(Y_block);
        dct(Cb_block);
        dct(Cr_block);
        
        // 3. BLOCKING WRITE: Push frequency blocks to Stage 4
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_block, 64);
    }
    
    printf("Stage 3: Finished transforming %d MCUs.\n", total_mcus);
    return 0;
}