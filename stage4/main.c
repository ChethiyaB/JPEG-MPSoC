#include "system.h"
#include <stdio.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"

#define FIFO_IN_BASE       FIFO_3_TO_4_OUT_BASE
#define FIFO_IN_CSR_BASE   FIFO_3_TO_4_OUT_CSR_BASE
#define FIFO_OUT_BASE      FIFO_4_TO_5_IN_BASE
#define FIFO_OUT_CSR_BASE  FIFO_4_TO_5_IN_CSR_BASE
#define FIFO_1_IN_BASE     FIFO_1_TO_4_OUT_BASE
#define FIFO_1_IN_CSR_BASE FIFO_1_TO_4_OUT_CSR_BASE

extern void quantization(INT16 *in, UINT16 *quant_table, INT16 *out);

UINT16 dynamic_q_table_luma[64];
UINT16 dynamic_q_table_chroma[64];

void receive_dqt_from_stage1() {
    INT16 receive_buffer[1];
    int i;
    for (i = 0; i < 64; i++) {
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, receive_buffer, 1);
        dynamic_q_table_luma[i] = (UINT16)receive_buffer[0];
    }
    for (i = 0; i < 64; i++) {
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, receive_buffer, 1);
        dynamic_q_table_chroma[i] = (UINT16)receive_buffer[0];
    }
}

int main() {
    printf("--- Nios II: Stage 4 (Quantization) ---\n");
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    fifo_init(FIFO_1_IN_CSR_BASE);

    receive_dqt_from_stage1();
    
    // --- DYNAMIC DIMENSION DAISY-CHAIN ---
    INT16 dimension_packet[2];
    fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, dimension_packet, 2);
    int width = (int)dimension_packet[0];
    int height = (int)dimension_packet[1];
    int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, dimension_packet, 2);
    // -------------------------------------

INT16 Y_in[64], Cb_in[64], Cr_in[64];
INT16 Y_out[64], Cb_out[64], Cr_out[64];

int i;
for (i = 0; i < total_mcus; i++) {
    fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_in, 64);
    fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_in, 64);
    fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_in, 64);
        
    quantization(Y_in, dynamic_q_table_luma, Y_out);
    quantization(Cb_in, dynamic_q_table_chroma, Cb_out);
    quantization(Cr_in, dynamic_q_table_chroma, Cr_out);

    if (i == 0) {
        int k;
        printf("Stage 4: MCU0 Y[0..7] after Quant: ");
        for (k = 0; k < 8; k++) printf("%d ", Y_out[k]);
        printf("\n");
        printf("Stage 4: MCU0 Cb[0..7] after Quant: ");
        for (k = 0; k < 8; k++) printf("%d ", Cb_out[k]);
        printf("\n");
        printf("Stage 4: MCU0 Cr[0..7] after Quant: ");
        for (k = 0; k < 8; k++) printf("%d ", Cr_out[k]);
        printf("\n");
    }
        
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_out, 64);
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_out, 64);
    fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_out, 64);
}
return 0;
}
