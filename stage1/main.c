#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/alt_timestamp.h>
#include "nios2_common/datatype.h"
#include "nios2_common/fifo_io.h"
#include "huffdata.h"
#include "encoder.h"

#define FIFO_OUT_TO_6_BASE      FIFO_1_TO_6_IN_BASE
#define FIFO_OUT_TO_6_CSR_BASE  FIFO_1_TO_6_IN_CSR_BASE
#define FIFO_OUT_BASE           FIFO_1_TO_2_IN_BASE
#define FIFO_OUT_CSR_BASE       FIFO_1_TO_2_IN_CSR_BASE
#define FIFO_OUT_TO_4_BASE      FIFO_1_TO_4_IN_BASE
#define FIFO_OUT_TO_4_CSR_BASE  FIFO_1_TO_4_IN_CSR_BASE
#define FIFO_OUT_TO_5_BASE      FIFO_1_TO_5_IN_BASE
#define FIFO_OUT_TO_5_CSR_BASE  FIFO_1_TO_5_IN_CSR_BASE

// --- GLOBAL BUFFERS TO PREVENT STACK OVERFLOW ---
UINT8 global_dynamic_header[623];
UINT8 global_strip_buffer[1536];
UINT8 global_row_data[128];
UINT8 global_scaled_q_luma[64];
UINT8 global_scaled_q_chroma[64];

// --- BUFFER FOR FAST-PATH SMALL IMAGES (32x32x3 = 3072 bytes) ---
UINT8 global_full_image_buffer[3072];

// --- TEMPLATES AND TABLES ---
const UINT8 full_jpeg_headers_template[623] = {
    0xFF, 0xD8,
    0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
    0xFF, 0xDB, 0x00, 0x43, 0x00,
    16, 11, 12, 14, 12, 10, 16, 14, 13, 14, 18, 17, 16, 19, 24, 40,
    26, 24, 22, 22, 24, 49, 35, 37, 29, 40, 58, 51, 61, 60, 57, 51,
    56, 55, 64, 72, 92, 78, 64, 68, 87, 69, 55, 56, 80, 109, 81, 87,
    95, 98, 103, 104, 103, 62, 77, 113, 121, 112, 100, 120, 92, 101, 103, 99,
    0xFF, 0xDB, 0x00, 0x43, 0x01,
    17, 18, 18, 24, 21, 24, 47, 26, 26, 47, 99, 66, 56, 66, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x18, 0x00, 0x18, 0x03,
    0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01,
    0xFF, 0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
    0xFF, 0xC4, 0x00, 0x1F, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
    0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00
};

const UINT8 q_table_luma[64] = {
    16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99
};

const UINT8 q_table_chroma[64] = {
    17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

const int ZIGZAG[64] = {
     0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 50, 56, 49, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 57,
    58, 52, 45, 38, 31, 39, 46, 53, 59, 60, 61, 54, 47, 55, 62, 63
};

int parse_image_header(FILE *file, int *width, int *height, int *pixel_offset, int *bpp) {
    unsigned char header[54];
    if (fread(header, 1, 54, file) < 54) { rewind(file); return 0; }
    if (header[0] == 'B' && header[1] == 'M') {
        *width  = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
        *height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
        *pixel_offset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
        *bpp = header[28] | (header[29] << 8);
        return 1;
    }
    rewind(file);
    return 0;
}

void send_dqt_to_stage4() {
    INT16 send_buffer[1];
    int i;
    for (i = 0; i < 64; i++) {
        send_buffer[0] = (INT16)global_scaled_q_luma[i];
        fifo_write_block(FIFO_OUT_TO_4_BASE, FIFO_OUT_TO_4_CSR_BASE, send_buffer, 1);
    }
    for (i = 0; i < 64; i++) {
        send_buffer[0] = (INT16)global_scaled_q_chroma[i];
        fifo_write_block(FIFO_OUT_TO_4_BASE, FIFO_OUT_TO_4_CSR_BASE, send_buffer, 1);
    }
}

void send_dht_to_stage5() {
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, luminance_dc_code_table, 12);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, luminance_dc_size_table, 12);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, chrominance_dc_code_table, 12);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, chrominance_dc_size_table, 12);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, luminance_ac_code_table, 256);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, luminance_ac_size_table, 256);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, chrominance_ac_code_table, 256);
    fifo_write_block(FIFO_OUT_TO_5_BASE, FIFO_OUT_TO_5_CSR_BASE, chrominance_ac_size_table, 256);
}

void send_output_filename_to_stage6(char *filename) {
    INT16 name_len = strlen(filename) + 1;
    fifo_write_block(FIFO_OUT_TO_6_BASE, FIFO_OUT_TO_6_CSR_BASE, &name_len, 1);

    int i;
    INT16 char_val;
    for (i = 0; i < name_len; i++) {
        char_val = (INT16)filename[i];
        fifo_write_block(FIFO_OUT_TO_6_BASE, FIFO_OUT_TO_6_CSR_BASE, &char_val, 1);
    }
}

void send_dynamic_header(int width, int height) {
    memcpy(global_dynamic_header, full_jpeg_headers_template, 623);

    int i;
    for(i = 0; i < 64; i++) {
        global_dynamic_header[25 + i] = global_scaled_q_luma[ZIGZAG[i]];
        global_dynamic_header[94 + i] = global_scaled_q_chroma[ZIGZAG[i]];
    }

    int sof_offset = 158;
    global_dynamic_header[sof_offset + 5] = (height >> 8) & 0xFF;
    global_dynamic_header[sof_offset + 6] = height & 0xFF;
    global_dynamic_header[sof_offset + 7] = (width >> 8) & 0xFF;
    global_dynamic_header[sof_offset + 8] = width & 0xFF;

    INT16 header_size = 623;
    fifo_write_block(FIFO_OUT_TO_6_BASE, FIFO_OUT_TO_6_CSR_BASE, &header_size, 1);

    INT16 send_buffer[1];
    for (i = 0; i < header_size; i++) {
        send_buffer[0] = (INT16)global_dynamic_header[i];
        fifo_write_block(FIFO_OUT_TO_6_BASE, FIFO_OUT_TO_6_CSR_BASE, send_buffer, 1);
    }
}


// --- MAIN FUNCTION ---
int main() {
    printf("--- Nios II: Stage 1 (Master Controller & File Config) ---\n\n");

    fifo_init(FIFO_OUT_CSR_BASE);
    fifo_init(FIFO_OUT_TO_4_CSR_BASE);
    fifo_init(FIFO_OUT_TO_5_CSR_BASE);
    fifo_init(FIFO_OUT_TO_6_CSR_BASE);

    char input_filename[64];
    char output_filename[64];
    int quality = 50;
    int num_images = 0;

    printf("Reading config file (/mnt/host/files/config.txt)...\n");
    FILE *f_cfg = fopen("/mnt/host/files/config.txt", "r");

    if (!f_cfg) {
        printf("ERROR: Could not open config file!\n");
        return -1;
    }

    // Read the total batch count first
    if (fscanf(f_cfg, "%d", &num_images) != 1) {
        printf("ERROR: Config file missing image count on line 1.\n");
        fclose(f_cfg);
        return -1;
    }

    printf("Batch Job Initialized: Processing %d images...\n", num_images);

    // --- LOOP THROUGH EVERY IMAGE ---
    int img_idx;
    for (img_idx = 0; img_idx < num_images; img_idx++) {
        if (fscanf(f_cfg, "%63s", input_filename) != 1 ||
            fscanf(f_cfg, "%63s", output_filename) != 1 ||
            fscanf(f_cfg, "%d", &quality) != 1) {
            printf("ERROR: Config file ran out of entries before expected!\n");
            break;
        }

        if (quality < 1) quality = 1;
        if (quality > 100) quality = 100;

        printf("\n============================================\n");
        printf("Processing Image %d of %d\n", img_idx + 1, num_images);
        printf("  Input: %s\n  Output: %s\n  Quality: %d\n", input_filename, output_filename, quality);

        FILE *f_in = fopen(input_filename, "rb");
        if (f_in == NULL) {
            printf("ERROR: Could not open input file %s!\n", input_filename);
            continue; // Skip to next image instead of crashing the pipeline
        }

        int width = 24, height = 24, pixel_offset = 0, bpp = 24;
        int is_bmp = parse_image_header(f_in, &width, &height, &pixel_offset, &bpp);

        if (is_bmp && bpp != 24) {
            printf("ERROR: Image is %d-bit. Must be 24-bit BMP. Skipping.\n", bpp);
            fclose(f_in);
            continue;
        }

        int scale;
        if (quality < 50) scale = 5000 / quality;
        else scale = 200 - quality * 2;

        int i, temp;
        for (i = 0; i < 64; i++) {
            temp = (q_table_luma[i] * scale + 50) / 100;
            if (temp < 1) temp = 1; if (temp > 255) temp = 255;
            global_scaled_q_luma[i] = (UINT8)temp;

            temp = (q_table_chroma[i] * scale + 50) / 100;
            if (temp < 1) temp = 1; if (temp > 255) temp = 255;
            global_scaled_q_chroma[i] = (UINT8)temp;
        }

        init_chroma_ac_tables();

        printf("Stage 1: Dispatching config data to downstream processors...\n");
        send_output_filename_to_stage6(output_filename);
        send_dqt_to_stage4();
        send_dht_to_stage5();
        send_dynamic_header(width, height);

        INT16 dimension_packet[2] = {(INT16)width, (INT16)height};
        fifo_write_block(FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, dimension_packet, 2);

        int mcus_x = (width + 7) / 8;
        int mcus_y = (height + 7) / 8;
        int row_padded = (width * 3 + 3) & (~3);

        // --- PRE-LOAD LOGIC FOR SMALL IMAGES ---
        int is_small_image = (width <= 32 && height <= 32);
        int y, c;

        if (is_small_image) {
            printf("Stage 1: Small image detected (%dx%d). Pre-loading to OCM...\n", width, height);
            for (y = 0; y < height; y++) {
                if (is_bmp) {
                    int bmp_row = (height - 1) - y;
                    fseek(f_in, pixel_offset + (bmp_row * row_padded), SEEK_SET);
                    fread(global_row_data, 1, row_padded, f_in);
                    for (c = 0; c < width; c++) {
                        global_full_image_buffer[(y * width + c) * 3 + 0] = global_row_data[c * 3 + 2];
                        global_full_image_buffer[(y * width + c) * 3 + 1] = global_row_data[c * 3 + 1];
                        global_full_image_buffer[(y * width + c) * 3 + 2] = global_row_data[c * 3 + 0];
                    }
                } else {
                    fseek(f_in, pixel_offset + (y * width * 3), SEEK_SET);
                    fread(&global_full_image_buffer[y * width * 3], 1, width * 3, f_in);
                }
            }
        }

        printf("Stage 1: Streaming image to pipeline...\n");

        // --- START TIMER NOW ---
        if (alt_timestamp_start() < 0) printf("Timer Error!\n");
        alt_u32 start_time = alt_timestamp();

        int mcu_y;
        for (mcu_y = 0; mcu_y < mcus_y; mcu_y++) {
            for (y = 0; y < 8; y++) {
                int global_y = mcu_y * 8 + y;
                if (global_y >= height) {
                    if (y > 0) memcpy(&global_strip_buffer[y * width * 3], &global_strip_buffer[(y - 1) * width * 3], width * 3);
                    continue;
                }

                if (is_small_image) {
                    memcpy(&global_strip_buffer[y * width * 3], &global_full_image_buffer[global_y * width * 3], width * 3);
                } else {
                    if (is_bmp) {
                        int bmp_row = (height - 1) - global_y;
                        fseek(f_in, pixel_offset + (bmp_row * row_padded), SEEK_SET);
                        fread(global_row_data, 1, row_padded, f_in);

                        for (c = 0; c < width; c++) {
                            global_strip_buffer[(y * width + c) * 3 + 0] = global_row_data[c * 3 + 2];
                            global_strip_buffer[(y * width + c) * 3 + 1] = global_row_data[c * 3 + 1];
                            global_strip_buffer[(y * width + c) * 3 + 2] = global_row_data[c * 3 + 0];
                        }
                    } else {
                        fseek(f_in, pixel_offset + (global_y * width * 3), SEEK_SET);
                        fread(&global_strip_buffer[y * width * 3], 1, width * 3, f_in);
                    }
                }
            }
            encode_mcu_row(global_strip_buffer, width, mcus_x, FIFO_OUT_BASE, FIFO_OUT_CSR_BASE, NULL);
        }

        // --- STOP TIMER ---
        alt_u32 end_time = alt_timestamp();

        // --- SAFE MATH TO PREVENT OVERFLOW ---
        alt_u32 ticks = end_time - start_time;
        if (ticks == 0) ticks = 1;

        alt_u32 freq = alt_timestamp_freq();
        float secs = (float)ticks / (float)freq;
        alt_u32 time_ms = (alt_u32)(secs * 1000.0f);

        int total_raw_bytes = width * height * 3;
        alt_u32 throughput = (alt_u32)((float)total_raw_bytes / secs);

        printf("--- STAGE 1 (DISPATCH) BENCHMARK ---\n");
        printf("Total Clock Cycles : %lu\n", ticks);
        printf("Total Time         : %lu milliseconds\n", time_ms);
        printf("Throughput         : %lu Raw Bytes/sec\n", throughput);

        fclose(f_in);
        printf("Stage 1: Dispatch complete for Image %d.\n", img_idx + 1);
    }

    fclose(f_cfg);

    // --- INFINITE TRAP ADDED HERE ---
    printf("\n============================================\n");
    printf("Batch processing complete! All images dispatched.\n");
    printf("Halting Master Controller to prevent loop restart.\n");
    printf("============================================\n");

    while(1) {
        // Infinite idle loop keeps the program from resetting to the start.
    }

    return 0;
}
