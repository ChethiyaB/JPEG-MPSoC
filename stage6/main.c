#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"

#define FIFO_1_IN_BASE       FIFO_1_TO_6_OUT_BASE
#define FIFO_1_IN_CSR_BASE   FIFO_1_TO_6_OUT_CSR_BASE
#define FIFO_5_IN_BASE       FIFO_5_TO_6_OUT_BASE
#define FIFO_5_IN_CSR_BASE   FIFO_5_TO_6_OUT_CSR_BASE

#define CHUNK_SIZE 4096

int main() {
    printf("--- Nios II: Stage 6 (Buffered File Writing) ---\n");
    
    fifo_init(FIFO_1_IN_CSR_BASE);
    fifo_init(FIFO_5_IN_CSR_BASE);
    INT16 buffer[1];

    // --- 1. GET FILENAME FROM STAGE 1 ---
    printf("Stage 6: Waiting for Output Filename from Stage 1...\n");

    // Read the string length first
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
    int name_len = buffer[0];
    
    // Extract the string character by character
    char output_filename[256];
    int i;
    for (i = 0; i < name_len; i++) {
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
        output_filename[i] = (char)(buffer[0] & 0xFF);
    }

    printf("Stage 6: Opening destination file: %s\n", output_filename);
    FILE *f_out = fopen(output_filename, "wb");
    if (!f_out) {
        printf("Stage 6 Error: Cannot create output file!\n");
        return -1;
    }

    // --- 2. GET PC HEADERS FROM STAGE 1 ---
    fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
    int header_size = buffer[0];

    for (i = 0; i < header_size; i++) {
        fifo_read_block(FIFO_1_IN_BASE, FIFO_1_IN_CSR_BASE, buffer, 1);
        fputc((UINT8)(buffer[0] & 0xFF), f_out);
    }
    printf("Stage 6: Headers written successfully.\n");

    // --- 3. STREAM AND BUFFER FILE DATA FROM STAGE 5 ---
    printf("Stage 6: Waiting for compressed bitstream from Stage 5...\n");
    
    int bytes_written = header_size;
    UINT16 prev_word = 0;
    UINT8 write_buffer[CHUNK_SIZE];
    int buf_idx = 0;

    while (1) {
        fifo_read_block(FIFO_5_IN_BASE, FIFO_5_IN_CSR_BASE, buffer, 1);
        UINT16 word = (UINT16)buffer[0];

        if (word == 0xAAAA && prev_word == 0xFFD9) {
            write_buffer[buf_idx++] = 0xFF;
            if (buf_idx >= CHUNK_SIZE) { fwrite(write_buffer, 1, CHUNK_SIZE, f_out); buf_idx = 0; }
            write_buffer[buf_idx++] = 0xD9;
            if (buf_idx >= CHUNK_SIZE) { fwrite(write_buffer, 1, CHUNK_SIZE, f_out); buf_idx = 0; }

            bytes_written += 2;
            printf("Stage 6: Detected EOI Handshake. File complete!\n");
            break;
        }

        if (word == 0xFFD9) {
            prev_word = word;
            continue;
        }

        UINT8 high_byte = (word >> 8) & 0xFF;
        UINT8 low_byte  = word & 0xFF;

        write_buffer[buf_idx++] = high_byte;
        if (buf_idx >= CHUNK_SIZE) { fwrite(write_buffer, 1, CHUNK_SIZE, f_out); buf_idx = 0; }
        write_buffer[buf_idx++] = low_byte;
        if (buf_idx >= CHUNK_SIZE) { fwrite(write_buffer, 1, CHUNK_SIZE, f_out); buf_idx = 0; }

        bytes_written += 2;
        prev_word = word;
    }

    // Flush remaining buffer
    if (buf_idx > 0) {
        fwrite(write_buffer, 1, buf_idx, f_out);
    }

    fclose(f_out);
    printf("Stage 6: Successfully wrote %d total bytes.\n", bytes_written);
    
    return 0;
}
