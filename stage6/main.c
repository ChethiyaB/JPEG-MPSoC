#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"
#include <sys/alt_timestamp.h>

#define FIFO_1_IN_BASE       FIFO_1_TO_6_OUT_BASE
#define FIFO_1_IN_CSR_BASE   FIFO_1_TO_6_OUT_CSR_BASE
#define FIFO_5_IN_BASE       FIFO_5_TO_6_OUT_BASE
#define FIFO_5_IN_CSR_BASE   FIFO_5_TO_6_OUT_CSR_BASE

// --- INCREASED BUFFER TO HOLD ENTIRE SMALL IMAGE (8KB) ---
#define MAX_FILE_SIZE 8192

// --- GLOBAL BUFFER TO PREVENT STACK OVERFLOW ---
UINT8 global_write_buffer[MAX_FILE_SIZE];
char global_output_filename[256];

int main() {
    printf("--- Nios II: Stage 6 (Buffered File Writing) ---\n");
    
    fifo_init(FIFO_1_IN_CSR_BASE);
    fifo_init(FIFO_5_IN_CSR_BASE);
    INT16 buffer[1];
    alt_u32 start_time, end_time, ticks;

    int image_counter = 1;

    // --- INFINITE BATCH PROCESSING LOOP ---
    while(1) {
        printf("\n============================================\n");
        printf("Stage 6: Ready and waiting for Image #%d...\n", image_counter++);

        // --- 1. GET FILENAME FROM STAGE 1 ---
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
        int name_len = buffer[0];

        int i;
        for (i = 0; i < name_len; i++) {
            fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
            global_output_filename[i] = (char)(buffer[0] & 0xFF);
        }
        printf("Stage 6: Output file will be: %s\n", global_output_filename);

        // --- 2. GET PC HEADERS FROM STAGE 1 (BUFFERED) ---
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
        int header_size = buffer[0];
        int buf_idx = 0; // RESET BUFFER POINTER FOR NEW IMAGE!

        // Save headers to OCM memory instead of directly to file
        for (i = 0; i < header_size; i++) {
            fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
            if (buf_idx < MAX_FILE_SIZE) {
                global_write_buffer[buf_idx++] = (UINT8)(buffer[0] & 0xFF);
            }
        }
        printf("Stage 6: Headers buffered into memory successfully.\n");

        // --- 3. STREAM AND BUFFER FILE DATA FROM STAGE 5 ---
        printf("Stage 6: Waiting for compressed bitstream from Stage 5...\n");

        int bytes_written = header_size;
        UINT16 prev_word = 0;

        // --- START TIMER HERE ---
        if (alt_timestamp_start() < 0) {
            printf("Timer Error: Check BSP settings!\n");
        }
        start_time = alt_timestamp();

        while (1) {
            fifo_read_block(FIFO_5_IN_BASE, FIFO_5_IN_CSR_BASE, buffer, 1);
            UINT16 word = (UINT16)buffer[0];

            if (word == 0xAAAA && prev_word == 0xFFD9) {
                if (buf_idx < MAX_FILE_SIZE) global_write_buffer[buf_idx++] = 0xFF;
                if (buf_idx < MAX_FILE_SIZE) global_write_buffer[buf_idx++] = 0xD9;
                bytes_written += 2;

                end_time = alt_timestamp();
                printf("Stage 6: Detected EOI Handshake. Pipeline sequence complete!\n");
                break;
            }

            if (word == 0xFFD9) {
                prev_word = word;
                continue;
            }

            UINT8 high_byte = (word >> 8) & 0xFF;
            UINT8 low_byte  = word & 0xFF;

            if (buf_idx < MAX_FILE_SIZE) global_write_buffer[buf_idx++] = high_byte;
            if (buf_idx < MAX_FILE_SIZE) global_write_buffer[buf_idx++] = low_byte;

            bytes_written += 2;
            prev_word = word;
        }

        // --- 4. WRITE ENTIRE ARRAY TO HARD DRIVE (OFF-CLOCK) ---
        printf("Stage 6: Writing memory buffer to HostFS...\n");
        FILE *f_out = fopen(global_output_filename, "wb");
        if (f_out) {
            if (buf_idx > 0) {
                fwrite(global_write_buffer, 1, buf_idx, f_out);
            }
            fclose(f_out);
        } else {
            printf("Stage 6 Error: Cannot create output file!\n");
        }

        // --- CALCULATE AND PRINT BENCHMARK RESULTS USING INTEGERS ---
        ticks = end_time - start_time;
        if (ticks == 0) ticks = 1;

        alt_u32 freq = alt_timestamp_freq();
        float secs_float = (float)ticks / (float)freq;

        alt_u32 time_ms = (alt_u32)(secs_float * 1000.0f);
        alt_u32 throughput = (alt_u32)((float)bytes_written / secs_float);

        printf("--- STAGE 6 (BITSTREAM OUTPUT) BENCHMARK ---\n");
        printf("Total Clock Cycles : %lu\n", ticks);
        printf("Total Time         : %lu milliseconds\n", time_ms);
        printf("Throughput         : %lu Compressed Bytes/sec\n", throughput);
        printf("Total Bytes Wrote  : %d\n", bytes_written);
    } // End of infinite while(1) loop

    return 0;
}
