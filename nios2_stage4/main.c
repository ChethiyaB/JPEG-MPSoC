#include <stdio.h>
#include "../nios2_common/datatype.h"
#include "../nios2_common/fifo_io.h"

#ifndef FIFO_IN_BASE
#define FIFO_IN_BASE     0x00000000
#define FIFO_IN_CSR_BASE 0x00000000
#endif

#ifndef FIFO_OUT_BASE
#define FIFO_OUT_BASE     0x00000000
#define FIFO_OUT_CSR_BASE 0x00000000
#endif

extern void initialize_quantization_tables(UINT32 quality, UINT16 *Lqt, UINT16 *Cqt);
extern void quantization(INT16 *in, UINT16 *quant_table, INT16 *out);

int main() {
    printf("--- Nios II: Stage 4 (Quantization & Zig-Zag) ---\n");
    
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    
    // Setup Quantization Tables (Quality = 90 to match Phase 0 golden reference)
    UINT16 Lqt[64], Cqt[64];
    initialize_quantization_tables(90, Lqt, Cqt);
    
    int width = 24, height = 24;
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    
    INT16 Y_in[64], Cb_in[64], Cr_in[64];
    INT16 Y_out[64], Cb_out[64], Cr_out[64];
    
    printf("Stage 4: Waiting for frequency blocks from Stage 3...\n");
    
    int i;
    for (i = 0; i < total_mcus; i++) {
        // 1. READ: Pull 2D frequency blocks from Stage 3
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_in, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_in, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_in, 64);
        
        // 2. PROCESS: Quantize and Zig-Zag
        quantization(Y_in, Lqt, Y_out);
        quantization(Cb_in, Cqt, Cb_out);
        quantization(Cr_in, Cqt, Cr_out);
        
        // 3. WRITE: Push 1D zig-zagged blocks to Stage 5
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_out, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_out, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_out, 64);
    }
    
    printf("Stage 4: Finished quantizing %d MCUs.\n", total_mcus);
    return 0;
}