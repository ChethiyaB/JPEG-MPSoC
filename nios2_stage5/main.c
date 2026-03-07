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

extern int huffman_encode_block(INT16 *block, INT16 *prev_dc, int is_chroma, INT16 *out_fifo_buffer);

int main() {
    printf("--- Nios II: Stage 5 (Huffman Encoding) ---\n");
    
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    
    int width = 24, height = 24;
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    
    INT16 Y_in[64], Cb_in[64], Cr_in[64];
    INT16 stream_buffer[256]; // Buffer to hold compressed words before sending
    
    // DC state
    INT16 prev_dc_y = 0, prev_dc_cb = 0, prev_dc_cr = 0;
    
    printf("Stage 5: Compressing data stream...\n");
    
    int i;
    for (i = 0; i < total_mcus; i++) {
        // 1. READ: Pull 1D zig-zagged blocks from Stage 4
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_in, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_in, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_in, 64);
        
        // 2. PROCESS & WRITE: Pack bits and push directly to Stage 6
        int words_to_send;
        
        words_to_send = huffman_encode_block(Y_in, &prev_dc_y, 0, stream_buffer);
        if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);
        
        words_to_send = huffman_encode_block(Cb_in, &prev_dc_cb, 1, stream_buffer);
        if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);
        
        words_to_send = huffman_encode_block(Cr_in, &prev_dc_cr, 1, stream_buffer);
        if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);
    }
    
    // Push the final End Of Image (EOI) marker 0xFFD9
    stream_buffer[0] = 0xFFD9;
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, 1);
    
    printf("Stage 5: Finished compressing %d MCUs and sent EOI.\n", total_mcus);
    return 0;
}