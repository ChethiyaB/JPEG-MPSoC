#include "system.h"
#include <stdio.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"

#define FIFO_IN_BASE       FIFO_1_TO_2_OUT_BASE
#define FIFO_IN_CSR_BASE   FIFO_1_TO_2_OUT_CSR_BASE
#define FIFO_OUT_BASE      FIFO_2_TO_3_IN_BASE
#define FIFO_OUT_CSR_BASE  FIFO_2_TO_3_IN_CSR_BASE

extern void levelshift(INT16 *block);

int main() {
    printf("--- Nios II: Stage 2 (Level Shifting) ---\n");
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    
    // --- DYNAMIC DIMENSION DAISY-CHAIN ---
    INT16 dimension_packet[2];
    fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, dimension_packet, 2);
    int width = (int)dimension_packet[0];
    int height = (int)dimension_packet[1];
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, dimension_packet, 2);
    // -------------------------------------

    INT16 Y_block[64], Cb_block[64], Cr_block[64];
    
    int i;
    for (i = 0; i < total_mcus; i++) {
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_block, 64);
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_block, 64);

        levelshift(Y_block);
        levelshift(Cb_block);
        levelshift(Cr_block);

        if (i == 0) {
            int k;
            printf("Stage 2: MCU0 Y[0..7] after levelshift: ");
            for (k = 0; k < 8; k++) {
                printf("%d ", Y_block[k]);
            }
            printf("\n");
        }

        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_block, 64);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_block, 64);
    }
    return 0;
}
