#include "system.h"
#include <stdio.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"
#include "huffdata.h"
#include <sys/alt_timestamp.h>

#define FIFO_IN_BASE       FIFO_4_TO_5_OUT_BASE
#define FIFO_IN_CSR_BASE   FIFO_4_TO_5_OUT_CSR_BASE
#define FIFO_OUT_BASE      FIFO_5_TO_6_IN_BASE
#define FIFO_OUT_CSR_BASE  FIFO_5_TO_6_IN_CSR_BASE
#define FIFO_1_IN_BASE     FIFO_1_TO_5_OUT_BASE
#define FIFO_1_IN_CSR_BASE FIFO_1_TO_5_OUT_CSR_BASE

extern int huffman_flush(INT16 *out_fifo_buffer);
extern int huffman_encode_block(INT16 *block, INT16 *prev_dc, int is_chroma, INT16 *out_fifo_buffer);

// --- MOVED TO GLOBAL MEMORY TO PREVENT STACK OVERFLOW ---
INT16 stream_buffer[256];

void receive_dht_from_stage1() {
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)luminance_dc_code_table, 12);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)luminance_dc_size_table, 12);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)chrominance_dc_code_table, 12);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)chrominance_dc_size_table, 12);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)luminance_ac_code_table, 256);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)luminance_ac_size_table, 256);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)chrominance_ac_code_table, 256);
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, (INT16*)chrominance_ac_size_table, 256);
}

int main() {
    printf("--- Nios II: Stage 5 (Huffman Encoding) ---\n");
    fifo_init(FIFO_IN_CSR_BASE);
    fifo_init(FIFO_OUT_CSR_BASE);
    fifo_init(FIFO_1_IN_CSR_BASE);

    int image_counter = 1;

    // --- INFINITE BATCH PROCESSING LOOP ---
    while (1) {
        printf("\nStage 5: Ready for Image #%d...\n", image_counter++);

        // Grab fresh tables from Master Controller for this specific image
        receive_dht_from_stage1();

        INT16 dimension_packet[2];
        fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, dimension_packet, 2);
        int width = (int)dimension_packet[0];
        int height = (int)dimension_packet[1];

        // 4:4:4 format as dictated by readYUV.c
        int total_mcus = ((width + 7) / 8) * ((height + 7) / 8);

        INT16 Y_in[64], Cb_in[64], Cr_in[64];

        // --- CRITICAL: PREDICTORS MUST BE RESET TO 0 FOR EVERY NEW IMAGE ---
        INT16 prev_dc_y = 0, prev_dc_cb = 0, prev_dc_cr = 0;

        // --- BENCHMARK VARIABLES ---
        alt_u32 start_time, end_time, ticks;

        printf("Stage 5: Starting MCU processing...\n");

        // --- START TIMER ---
        if (alt_timestamp_start() < 0) {
            printf("Timer Error: Check BSP settings!\n");
        }
        start_time = alt_timestamp();

        int i, words_to_send;
        for (i = 0; i < total_mcus; i++) {
            // Read the perfectly zig-zagged blocks directly from Stage 4
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Y_in, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cb_in, 64);
            fifo_read_block(FIFO_IN_BASE, FIFO_IN_CSR_BASE, Cr_in, 64);

            // Pass them straight into the encoder!
            words_to_send = huffman_encode_block(Y_in, &prev_dc_y, 0, stream_buffer);
            if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);

            words_to_send = huffman_encode_block(Cb_in, &prev_dc_cb, 1, stream_buffer);
            if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);

            words_to_send = huffman_encode_block(Cr_in, &prev_dc_cr, 1, stream_buffer);
            if (words_to_send > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, words_to_send);
        }

        // Flush any remaining bits in the Huffman buffer
        int flush_words = huffman_flush(stream_buffer);
        if (flush_words > 0) fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, flush_words);

        // --- STOP CLOCK ---
        end_time = alt_timestamp();

        // --- SAFE MATH TO PREVENT OVERFLOW ---
        ticks = end_time - start_time;
        if (ticks == 0) ticks = 1;
        alt_u32 freq = alt_timestamp_freq();
        float secs = (float)ticks / (float)freq;
        alt_u32 time_ms = (alt_u32)(secs * 1000.0f);

        alt_u32 throughput = (alt_u32)((float)total_mcus / secs);

        printf("--- STAGE 5 (HUFFMAN) BENCHMARK ---\n");
        printf("Total Clock Cycles : %lu\n", ticks);
        printf("Total Time         : %lu ms\n", time_ms);
        printf("Throughput         : %lu MCUs/sec\n", throughput); // Using Int format

        // Send EOI and Handshake to Stage 6
        stream_buffer[0] = 0xFFD9;
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, 1);

        stream_buffer[0] = 0xAAAA;
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, stream_buffer, 1);
    }

    return 0;
}
