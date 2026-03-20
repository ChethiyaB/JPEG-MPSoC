#include "encoder.h"
#include "nios2_common/fifo_io.h"

void encode_mcu_row(UINT8 *strip_buffer, int width, int mcus_x, unsigned int fifo_base, unsigned int fifo_csr, FILE *f_stage1) {
    INT16 Y_block[64], Cb_block[64], Cr_block[64];
    int col, x, y;

    for (col = 0; col < mcus_x; col++) {

        // 1. Safely extract the 8x8 block from our tiny strip buffer
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                int global_x = (col * 8) + x;

                // Clamp to the edge if the image width isn't a perfect multiple of 8
                if (global_x >= width) {
                    global_x = width - 1;
                }

                // Since our buffer is only 8 rows tall, 'y' is our exact row index!
                int pixel_idx = (y * width + global_x) * 3;

                INT16 R = strip_buffer[pixel_idx];
                INT16 G = strip_buffer[pixel_idx + 1];
                INT16 B = strip_buffer[pixel_idx + 2];

                // 2. RGB to YCbCr Math
                int Y_val  = ( 19595 * R + 38469 * G +  7471 * B) >> 16;
                int Cb_val = ((-11059 * R - 21709 * G + 32768 * B) >> 16) + 128;
                int Cr_val = (( 32768 * R - 27439 * G -  5329 * B) >> 16) + 128;

                Y_block[y * 8 + x]  = (INT16)Y_val;
                Cb_block[y * 8 + x] = (INT16)Cb_val;
                Cr_block[y * 8 + x] = (INT16)Cr_val;
            }
        }

        // Dump Stage 1 MCU to host for comparison (if file is open)
        if (f_stage1) {
            fwrite(Y_block, sizeof(INT16), 64, f_stage1);
            fwrite(Cb_block, sizeof(INT16), 64, f_stage1);
            fwrite(Cr_block, sizeof(INT16), 64, f_stage1);
        }

        // 3. Push Block to Hardware FIFO (Stage 2)
        fifo_write_block(fifo_base, fifo_csr, Y_block, 64);
        fifo_write_block(fifo_base, fifo_csr, Cb_block, 64);
        fifo_write_block(fifo_base, fifo_csr, Cr_block, 64);
    }
}
