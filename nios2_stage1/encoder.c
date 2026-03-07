#include "encoder.h"
#include "../nios2_common/fifo_io.h"

extern void read_444_format(UINT8 *input, int width, int height, INT16 *Y, INT16 *CB, INT16 *CR, int mcu_col, int mcu_row);

void encode_image_stage1(UINT8 *rgb_in, int width, int height) {
    int mcus_x = (width + 7) / 8;
    int mcus_y = (height + 7) / 8;
    
    INT16 Y_block[64], Cb_block[64], Cr_block[64];
    
    int row, col;
    for (row = 0; row < mcus_y; row++) {
        for (col = 0; col < mcus_x; col++) {
            
            // 1. Extract and convert an 8x8 block
            read_444_format(rgb_in, width, height, Y_block, Cb_block, Cr_block, col, row);
            
            // 2. Push converted blocks to Stage 2 via FIFO
            // The fifo_write_block function will stall CPU 1 if the FIFO is full
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_block, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_block, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_block, 64);
        }
    }
}