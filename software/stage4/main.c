#include "system.h"
#include <stdio.h>
#include <sys/alt_timestamp.h>
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

    int image_counter = 1;

    // --- INFINITE BATCH PROCESSING LOOP ---
    while (1) {
        printf("\nStage 4: Ready for Image #%d...\n", image_counter++);

        // Grab fresh tables from Master Controller for this specific image
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

        //---START TIMER---
        if (alt_timestamp_start() < 0) printf("Timer Error!\n");
        alt_u32 start_time = alt_timestamp();

        int i;
        for (i = 0; i < total_mcus; i++) {
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_in, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_in, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_in, 64);

            quantization(Y_in, dynamic_q_table_luma, Y_out);
            quantization(Cb_in, dynamic_q_table_chroma, Cb_out);
            quantization(Cr_in, dynamic_q_table_chroma, Cr_out);

            // --- FIXED: WRITE IMMEDIATELY TO STAGE 5 ---
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_out, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_out, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_out, 64);
        }

        //---STOP TIMER---
        alt_u32 end_time = alt_timestamp();
        alt_u32 ticks = end_time - start_time;
        if (ticks == 0) ticks = 1;

        // --- SAFE MATH TO PREVENT OVERFLOW ---
        alt_u32 freq = alt_timestamp_freq();
        float secs = (float)ticks / (float)freq;
        alt_u32 time_ms = (alt_u32)(secs * 1000.0f);

        int total_raw_bytes = width * height * 3;
        alt_u32 throughput = (alt_u32)((float)total_raw_bytes / secs);

        printf("--- STAGE 4 BENCHMARK ---\n");
        printf("Total Clock Cycles : %lu\n", ticks);
        printf("Total Time         : %lu milliseconds\n", time_ms);
        printf("Throughput         : %lu Raw Bytes/sec\n", throughput);
    }

    return 0;
}
