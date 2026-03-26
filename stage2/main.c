#include "system.h"
#include <stdio.h>
#include <sys/alt_timestamp.h>
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
    
    int image_counter = 1;

    // --- INFINITE BATCH PROCESSING LOOP ---
    while (1) {
        printf("\nStage 2: Ready for Image #%d...\n", image_counter++);

        // --- DYNAMIC DIMENSION DAISY-CHAIN ---
        INT16 dimension_packet[2];
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, dimension_packet, 2);
        int width = (int)dimension_packet[0];
        int height = (int)dimension_packet[1];
        int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, dimension_packet, 2);
        // -------------------------------------

        INT16 Y_block[64], Cb_block[64], Cr_block[64];

        //---START TIMER---
        if (alt_timestamp_start() < 0) printf("Timer Error!\n");
        alt_u32 start_time = alt_timestamp();

        int i;
        for (i = 0; i < total_mcus; i++) {
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_block, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_block, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_block, 64);

            levelshift(Y_block);
            levelshift(Cb_block);
            levelshift(Cr_block);

            // --- FIXED: WRITE IMMEDIATELY TO STAGE 3 ---
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Y_block, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cb_block, 64);
            fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, Cr_block, 64);
        }

        //---STOP TIMER---
        alt_u32 end_time = alt_timestamp();
        alt_u32 ticks = end_time - start_time;
        if (ticks == 0) ticks = 1;

        // --- SAFE MATH TO PREVENT OVERFLOW & PRINT GARBAGE ---
        alt_u32 freq = alt_timestamp_freq();
        float secs = (float)ticks / (float)freq;
        alt_u32 time_ms = (alt_u32)(secs * 1000.0f); // Cast to integer for %lu

        int total_raw_bytes = width * height * 3;
        alt_u32 throughput = (alt_u32)((float)total_raw_bytes / secs);

        printf("--- STAGE 2 BENCHMARK ---\n");
        printf("Total Clock Cycles : %lu\n", ticks);
        printf("Total Time         : %lu milliseconds\n", time_ms);
        printf("Throughput         : %lu Raw Bytes/sec\n", throughput);
    }

    return 0;
}
