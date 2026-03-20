// Software-only twin of the 6‑stage JPEG encoder pipeline.
// It reuses the same math and tables as the Nios stages, but replaces
// FIFOs and Nios-specific headers with in-memory arrays and a single main().

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Reuse the fixed-width types from the existing codebase
#include "stage1/nios2_common/datatype.h"
#include "stage5/huffdata.h"

// Stage 2 / 3 / 4 / 5 core functions we will link against
extern void levelshift(INT16 *block);                                            // stage2/levelshift.c
extern void dct(INT16 *block);                                                  // stage3/dct.c
extern void quantization(INT16 *in, UINT16 *quant_table, INT16 *out);           // stage4/quant.c
extern void initialize_quantization_tables(UINT32 quality, UINT16 *Lqt, UINT16 *Cqt); // stage4/quant.c
extern int huffman_encode_block(INT16 *block, INT16 *prev_dc, int is_chroma,
                                INT16 *out_fifo_buffer);                        // stage5/huffman.c
extern int huffman_flush(INT16 *out_fifo_buffer);                               // stage5/huffman.c
extern void init_chroma_ac_tables(void);                                        // stage5/huffdata.c (also called by stage1)
extern const UINT16 zigzag_table[64];                                           // stage4/quantdata.c

// ---------------------------------------------------------------------------
// Templates and tables (copied from stage1/main.c for fidelity)
// ---------------------------------------------------------------------------

// The Header template used on hardware (already zig‑zagged DQT tables)
static const UINT8 full_jpeg_headers_template[623] = {
    // SOI (Start of Image)
    0xFF, 0xD8,
    // APP0 (JFIF Header)
    0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x00,

    // DQT0 (Luma - Zig-Zagged for PC Decoder)
    0xFF, 0xDB, 0x00, 0x43, 0x00,
    16, 11, 12, 14, 12, 10, 16, 14, 13, 14, 18, 17, 16, 19, 24, 40,
    26, 24, 22, 22, 24, 49, 35, 37, 29, 40, 58, 51, 61, 60, 57, 51,
    56, 55, 64, 72, 92, 78, 64, 68, 87, 69, 55, 56, 80, 109, 81, 87,
    95, 98, 103, 104, 103, 62, 77, 113, 121, 112, 100, 120, 92, 101,
    103, 99,

    // DQT1 (Chroma - Zig-Zagged for PC Decoder)
    0xFF, 0xDB, 0x00, 0x43, 0x01,
    17, 18, 18, 24, 21, 24, 47, 26, 26, 47, 99, 66, 56, 66, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,

    // SOF0 (Frame Header) - Starts at index 158
    0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x18, 0x00, 0x18, 0x03,
    0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01,
    // DHT0_DC
    0xFF, 0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B,
    // DHT0_AC
    0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03,
    0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01,
    0x7D,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
    0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32,
    0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
    0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
    0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94,
    0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
    0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
    0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
    0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
    0xF9, 0xFA,
    // DHT1_DC
    0xFF, 0xC4, 0x00, 0x1F, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B,
    // DHT1_AC
    0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04,
    0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02,
    0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06,
    0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81,
    0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
    0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
    0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
    0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
    0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
    0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
    0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
    0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
    0xF9, 0xFA,
    // SOS (Start of Scan)
    0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11,
    0x03, 0x11, 0x00, 0x3F, 0x00
};

// Standard Q50 Luminance Table (Row-Major for Stage 4 Hardware Math)
static const UINT8 q_table_luma[64] = {
    16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99
};

// Standard Q50 Chrominance Table (Row-Major for Stage 4 Hardware Math)
static const UINT8 q_table_chroma[64] = {
    17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
};

// ---------------------------------------------------------------------------
// Helper: BMP / RAW header parsing (copied from stage1/main.c)
// ---------------------------------------------------------------------------

// Returns 1 if BMP, 0 if RAW (height/width left as defaults)
static int parse_image_header(FILE *file, int *width, int *height,
                              int *pixel_offset, int *bpp) {
    unsigned char header[54];

    if (fread(header, 1, 54, file) < 54) {
        rewind(file);
        return 0;
    }

    if (header[0] == 'B' && header[1] == 'M') {
        *width  = header[18] | (header[19] << 8) |
                  (header[20] << 16) | (header[21] << 24);
        *height = header[22] | (header[23] << 8) |
                  (header[24] << 16) | (header[25] << 24);
        *pixel_offset = header[10] | (header[11] << 8) |
                        (header[12] << 16) | (header[13] << 24);
        *bpp = header[28] | (header[29] << 8);
        return 1;
    }

    rewind(file);
    return 0;
}

// Build a dynamic header identical to Stage 1's behaviour
static int build_dynamic_header(int width, int height,
                                UINT8 *out_header, int out_capacity) {
    if (out_capacity < (int)sizeof(full_jpeg_headers_template)) {
        return -1;
    }
    memcpy(out_header, full_jpeg_headers_template,
           sizeof(full_jpeg_headers_template));

    // Patch SOF0 height/width (big-endian) at offset 158+5..8
    int sof_offset = 158;
    out_header[sof_offset + 5] = (height >> 8) & 0xFF;
    out_header[sof_offset + 6] = height & 0xFF;
    out_header[sof_offset + 7] = (width >> 8) & 0xFF;
    out_header[sof_offset + 8] = width & 0xFF;

    return (int)sizeof(full_jpeg_headers_template);
}

static int clamp_quality_int(int q) {
    if (q < 1) return 1;
    if (q > 100) return 100;
    return q;
}

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void prompt_line(const char *prompt, const char *default_val,
                        char *out, size_t out_cap) {
    printf("%s [%s]: ", prompt, default_val);
    fflush(stdout);
    if (!fgets(out, (int)out_cap, stdin)) {
        // No stdin, fall back to default
        strncpy(out, default_val, out_cap);
        out[out_cap - 1] = '\0';
        printf("\n");
        return;
    }
    trim_newline(out);
    if (out[0] == '\0') {
        strncpy(out, default_val, out_cap);
        out[out_cap - 1] = '\0';
    }
}

static int prompt_int(const char *prompt, int default_val) {
    char buf[32];
    char defbuf[16];
    snprintf(defbuf, sizeof(defbuf), "%d", default_val);
    prompt_line(prompt, defbuf, buf, sizeof(buf));
    int v = atoi(buf);
    if (v == 0) return default_val;
    return v;
}

static int patch_dqt_tables_in_header(UINT8 *header, int header_size,
                                      const UINT8 *luma_zz, const UINT8 *chroma_zz) {
    // Find DQT0 marker (FF DB 00 43 00) and DQT1 marker (FF DB 00 43 01)
    // and overwrite the following 64 bytes.
    const UINT8 dqt0_sig[5] = {0xFF, 0xDB, 0x00, 0x43, 0x00};
    const UINT8 dqt1_sig[5] = {0xFF, 0xDB, 0x00, 0x43, 0x01};

    int found0 = 0, found1 = 0;
    int i;
    for (i = 0; i + 5 + 64 <= header_size; i++) {
        if (!found0 && memcmp(&header[i], dqt0_sig, 5) == 0) {
            memcpy(&header[i + 5], luma_zz, 64);
            found0 = 1;
            continue;
        }
        if (!found1 && memcmp(&header[i], dqt1_sig, 5) == 0) {
            memcpy(&header[i + 5], chroma_zz, 64);
            found1 = 1;
            continue;
        }
        if (found0 && found1) break;
    }
    return (found0 && found1) ? 0 : -1;
}

// ---------------------------------------------------------------------------
// Stage 1 logic: RGB -> YCbCr MCUs in memory (no FIFOs)
// ---------------------------------------------------------------------------

static void convert_rgb_to_ycbcr_blocks(const UINT8 *rgb_in,
                                        int width, int height,
                                        INT16 *Y_blocks,
                                        INT16 *Cb_blocks,
                                        INT16 *Cr_blocks) {
    int mcus_x = (width + 7) / 8;
    int mcus_y = (height + 7) / 8;

    int row, col, x, y;
    for (row = 0; row < mcus_y; row++) {
        for (col = 0; col < mcus_x; col++) {
            int mcu_index = row * mcus_x + col;
            INT16 *Y_block  = &Y_blocks[mcu_index * 64];
            INT16 *Cb_block = &Cb_blocks[mcu_index * 64];
            INT16 *Cr_block = &Cr_blocks[mcu_index * 64];

            for (y = 0; y < 8; y++) {
                for (x = 0; x < 8; x++) {
                    int global_x = col * 8 + x;
                    int global_y = row * 8 + y;

                    if (global_x >= width)  global_x = width - 1;
                    if (global_y >= height) global_y = height - 1;

                    int pixel_idx = (global_y * width + global_x) * 3;

                    int R = rgb_in[pixel_idx + 0];
                    int G = rgb_in[pixel_idx + 1];
                    int B = rgb_in[pixel_idx + 2];

                    int Y  = ( 19595 * R + 38469 * G +  7471 * B) >> 16;
                    int Cb = ((-11059 * R - 21709 * G + 32768 * B) >> 16) + 128;
                    int Cr = (( 32768 * R - 27439 * G -  5329 * B) >> 16) + 128;

                    int blk_idx = y * 8 + x;
                    Y_block[blk_idx]  = (INT16)Y;
                    Cb_block[blk_idx] = (INT16)Cb;
                    Cr_block[blk_idx] = (INT16)Cr;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Huffman + file-level byte packing (Stage 5 + Stage 6 combined)
// ---------------------------------------------------------------------------

// Encodes all MCUs using the existing Huffman core and writes a contiguous
// byte stream into out_bytes.
//
// This mirrors the UPDATED Stage 5 + Stage 6 behavior in your repo:
// - Stage 5 outputs 16-bit words (with 0xFF byte stuffing already applied)
// - Then it sends an EOI handshake: 0xFFD9 followed by 0xAAAA
// - Stage 6 does NOT write the 0xFFD9 word directly; it waits for 0xAAAA after it,
//   then writes the true EOI bytes 0xFF 0xD9 once and stops.
static size_t encode_bitstream(INT16 *Y_q, INT16 *Cb_q, INT16 *Cr_q,
                               int total_mcus,
                               UINT8 *out_bytes, size_t out_capacity) {
    INT16 stream_buffer[256];
    INT16 prev_dc_y = 0, prev_dc_cb = 0, prev_dc_cr = 0;
    size_t out_len = 0;

    // We mimic the FIFO words Stage 5 would send, but directly unpack them
    // to bytes using Stage 6's handshake rules.
    UINT16 prev_word = 0;

    int i;
    for (i = 0; i < total_mcus; i++) {
        int words_to_send;

        words_to_send = huffman_encode_block(&Y_q[i * 64], &prev_dc_y, 0,
                                             stream_buffer);
        if (words_to_send > 0) {
            int w;
            for (w = 0; w < words_to_send; w++) {
                UINT16 word = (UINT16)stream_buffer[w];

                // Stage 6 behavior: if it ever sees the handshake, it would stop.
                if (word == 0xAAAA && prev_word == 0xFFD9) {
                    if (out_len + 2 > out_capacity) return out_len;
                    out_bytes[out_len++] = 0xFF;
                    out_bytes[out_len++] = 0xD9;
                    return out_len;
                }
                if (word == 0xFFD9) {
                    prev_word = word;
                    continue;
                }

                UINT8 high = (UINT8)((word >> 8) & 0xFF);
                UINT8 low  = (UINT8)(word & 0xFF);
                if (out_len + 2 > out_capacity) return out_len;
                out_bytes[out_len++] = high;
                out_bytes[out_len++] = low;

                prev_word = word;
            }
        }

        words_to_send = huffman_encode_block(&Cb_q[i * 64], &prev_dc_cb, 1,
                                             stream_buffer);
        if (words_to_send > 0) {
            int w;
            for (w = 0; w < words_to_send; w++) {
                UINT16 word = (UINT16)stream_buffer[w];

                if (word == 0xAAAA && prev_word == 0xFFD9) {
                    if (out_len + 2 > out_capacity) return out_len;
                    out_bytes[out_len++] = 0xFF;
                    out_bytes[out_len++] = 0xD9;
                    return out_len;
                }
                if (word == 0xFFD9) {
                    prev_word = word;
                    continue;
                }

                UINT8 high = (UINT8)((word >> 8) & 0xFF);
                UINT8 low  = (UINT8)(word & 0xFF);
                if (out_len + 2 > out_capacity) return out_len;
                out_bytes[out_len++] = high;
                out_bytes[out_len++] = low;

                prev_word = word;
            }
        }

        words_to_send = huffman_encode_block(&Cr_q[i * 64], &prev_dc_cr, 1,
                                             stream_buffer);
        if (words_to_send > 0) {
            int w;
            for (w = 0; w < words_to_send; w++) {
                UINT16 word = (UINT16)stream_buffer[w];

                if (word == 0xAAAA && prev_word == 0xFFD9) {
                    if (out_len + 2 > out_capacity) return out_len;
                    out_bytes[out_len++] = 0xFF;
                    out_bytes[out_len++] = 0xD9;
                    return out_len;
                }
                if (word == 0xFFD9) {
                    prev_word = word;
                    continue;
                }

                UINT8 high = (UINT8)((word >> 8) & 0xFF);
                UINT8 low  = (UINT8)(word & 0xFF);
                if (out_len + 2 > out_capacity) return out_len;
                out_bytes[out_len++] = high;
                out_bytes[out_len++] = low;

                prev_word = word;
            }
        }
    }

    // Flush remaining bits
    int flush_words = huffman_flush(stream_buffer);
    if (flush_words > 0) {
        int w;
        for (w = 0; w < flush_words; w++) {
            UINT16 word = (UINT16)stream_buffer[w];

            if (word == 0xAAAA && prev_word == 0xFFD9) {
                if (out_len + 2 > out_capacity) return out_len;
                out_bytes[out_len++] = 0xFF;
                out_bytes[out_len++] = 0xD9;
                return out_len;
            }
            if (word == 0xFFD9) {
                prev_word = word;
                continue;
            }

            UINT8 high = (UINT8)((word >> 8) & 0xFF);
            UINT8 low  = (UINT8)(word & 0xFF);
            if (out_len + 2 > out_capacity) return out_len;
            out_bytes[out_len++] = high;
            out_bytes[out_len++] = low;

            prev_word = word;
        }
    }

    // Mirror Stage 5's explicit 2-word handshake at end of stream:
    // word 0xFFD9 then word 0xAAAA, and Stage 6 turns that into bytes 0xFF 0xD9.
    if (out_len + 2 <= out_capacity) {
        // Process "word == 0xFFD9" (Stage 6 would store and not write)
        prev_word = 0xFFD9;
        // Process "word == 0xAAAA after prev 0xFFD9"
        out_bytes[out_len++] = 0xFF;
        out_bytes[out_len++] = 0xD9;
    }

    return out_len;
}

// ---------------------------------------------------------------------------
// Main: End‑to‑end software twin
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
    char in_path_buf[256] = "./test.bmp";
    char out_path_buf[256] = "./test_sw_twin.jpg";
    int quality = 50; // JPEG quality factor: 1..100 (higher = better quality, larger file)

    // CLI: sw_twin [input.bmp] [output.jpg] [quality]
    // If any are missing, we prompt interactively (like the hardware apps).
    if (argc >= 2) {
        strncpy(in_path_buf, argv[1], sizeof(in_path_buf));
        in_path_buf[sizeof(in_path_buf) - 1] = '\0';
    } else {
        prompt_line("Input image path", in_path_buf, in_path_buf, sizeof(in_path_buf));
    }

    if (argc >= 3) {
        strncpy(out_path_buf, argv[2], sizeof(out_path_buf));
        out_path_buf[sizeof(out_path_buf) - 1] = '\0';
    } else {
        prompt_line("Output JPEG path", out_path_buf, out_path_buf, sizeof(out_path_buf));
    }

    if (argc >= 4) {
        quality = atoi(argv[3]);
    } else {
        quality = prompt_int("JPEG quality (1-100)", quality);
    }
    quality = clamp_quality_int(quality);

    const char *in_path = in_path_buf;
    const char *out_path = out_path_buf;

    printf("[SW TWIN] Opening input image: %s\n", in_path);
    printf("[SW TWIN] Quality: %d\n", quality);
    FILE *f_in = fopen(in_path, "rb");
    if (!f_in) {
        printf("[SW TWIN] ERROR: cannot open input file.\n");
        return -1;
    }

    // Defaults for .raw style input, will be overwritten for BMP
    int width = 24, height = 24, pixel_offset = 0, bpp = 24;
    int is_bmp = parse_image_header(f_in, &width, &height,
                                    &pixel_offset, &bpp);

    if (is_bmp) {
        printf("[SW TWIN] Detected BMP: %dx%d, %d bpp\n", width, height, bpp);
        if (bpp != 24) {
            printf("[SW TWIN] FATAL: Only 24‑bit BMP is supported.\n");
            fclose(f_in);
            return -1;
        }
    } else {
        printf("[SW TWIN] No BMP header; assuming raw RGB %dx%d\n",
               width, height);
    }

    int raw_size = width * height * 3;
    UINT8 *rgb_buffer = (UINT8*)calloc(raw_size, sizeof(UINT8));
    if (!rgb_buffer) {
        printf("[SW TWIN] ERROR: cannot allocate RGB buffer.\n");
        fclose(f_in);
        return -1;
    }

    if (is_bmp) {
        fseek(f_in, pixel_offset, SEEK_SET);
        int row_padded = (width * 3 + 3) & (~3);
        UINT8 *row_data = (UINT8*)malloc(row_padded);
        if (!row_data) {
            printf("[SW TWIN] ERROR: cannot allocate row buffer.\n");
            free(rgb_buffer);
            fclose(f_in);
            return -1;
        }
        int r, c;
        for (r = height - 1; r >= 0; r--) {
            fread(row_data, 1, row_padded, f_in);
            for (c = 0; c < width; c++) {
                rgb_buffer[(r * width + c) * 3 + 0] = row_data[c * 3 + 2]; // R
                rgb_buffer[(r * width + c) * 3 + 1] = row_data[c * 3 + 1]; // G
                rgb_buffer[(r * width + c) * 3 + 2] = row_data[c * 3 + 0]; // B
            }
        }
        free(row_data);
    } else {
        fread(rgb_buffer, 1, raw_size, f_in);
    }
    fclose(f_in);

    printf("[SW TWIN] Sample RGB pixels: "
           "[%d,%d,%d] [%d,%d,%d] [%d,%d,%d]\n",
           rgb_buffer[0], rgb_buffer[1], rgb_buffer[2],
           rgb_buffer[3], rgb_buffer[4], rgb_buffer[5],
           rgb_buffer[6], rgb_buffer[7], rgb_buffer[8]);

    int mcus_x = (width + 7) / 8;
    int mcus_y = (height + 7) / 8;
    int total_mcus = mcus_x * mcus_y;

    printf("[SW TWIN] total MCUs: %d (%d x %d)\n",
           total_mcus, mcus_x, mcus_y);

    int i; // loop/debug variable used below

    INT16 *Y_blocks  = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    INT16 *Cb_blocks = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    INT16 *Cr_blocks = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    if (!Y_blocks || !Cb_blocks || !Cr_blocks) {
        printf("[SW TWIN] ERROR: cannot allocate Y/Cb/Cr blocks.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        return -1;
    }

    // Stage 1 (software version)
    convert_rgb_to_ycbcr_blocks(rgb_buffer, width, height,
                                Y_blocks, Cb_blocks, Cr_blocks);

    // Debug: match Stage 1 MCU 0 Y-block printout
    printf("\n[SW TWIN] --- Stage 1: MCU 0 (Y-Block) BEFORE Level Shift ---\n");
    for (i = 0; i < 64; i++) {
        printf("%6d ", Y_blocks[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }

    // Stage 2: level shift (in-place)
    for (i = 0; i < total_mcus; i++) {
        levelshift(&Y_blocks[i * 64]);
        levelshift(&Cb_blocks[i * 64]);
        levelshift(&Cr_blocks[i * 64]);
    }

    // Debug: match Stage 2 log
    printf("[SW TWIN] Stage 2: MCU0 Y[0..7] after levelshift: ");
    for (i = 0; i < 8; i++) {
        printf("%d ", Y_blocks[i]);
    }
    printf("\n");

    // Stage 3: DCT (in-place)
    for (i = 0; i < total_mcus; i++) {
        dct(&Y_blocks[i * 64]);
        dct(&Cb_blocks[i * 64]);
        dct(&Cr_blocks[i * 64]);
    }

    // Debug: match Stage 3 log
    printf("[SW TWIN] Stage 3: MCU0 Y[0..7] after DCT: ");
    for (i = 0; i < 8; i++) {
        printf("%d ", Y_blocks[i]);
    }
    printf("\n");

    // Stage 4: Quantization (separate arrays, as quantization() is not in-place)
    // Generate quantization tables from quality factor, matching stage4/quant.c.
    UINT16 q_luma16[64], q_chroma16[64];
    initialize_quantization_tables((UINT32)quality, q_luma16, q_chroma16);

    INT16 *Y_q  = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    INT16 *Cb_q = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    INT16 *Cr_q = (INT16*)malloc(total_mcus * 64 * sizeof(INT16));
    if (!Y_q || !Cb_q || !Cr_q) {
        printf("[SW TWIN] ERROR: cannot allocate quantized blocks.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        free(Y_q); free(Cb_q); free(Cr_q);
        return -1;
    }

    for (i = 0; i < total_mcus; i++) {
        quantization(&Y_blocks[i * 64],  q_luma16,   &Y_q[i * 64]);
        quantization(&Cb_blocks[i * 64], q_chroma16, &Cb_q[i * 64]);
        quantization(&Cr_blocks[i * 64], q_chroma16, &Cr_q[i * 64]);
    }

    // Debug: match Stage 4 log
    printf("[SW TWIN] Stage 4: MCU0 Y[0..7] after Quant: ");
    for (i = 0; i < 8; i++) printf("%d ", Y_q[i]);
    printf("\n");
    printf("[SW TWIN] Stage 4: MCU0 Cb[0..7] after Quant: ");
    for (i = 0; i < 8; i++) printf("%d ", Cb_q[i]);
    printf("\n");
    printf("[SW TWIN] Stage 4: MCU0 Cr[0..7] after Quant: ");
    for (i = 0; i < 8; i++) printf("%d ", Cr_q[i]);
    printf("\n");

    // Stage 5 + 6: Huffman + bitstream packing
    // IMPORTANT: In your updated pipeline Stage 1 calls init_chroma_ac_tables()
    // to populate the 256-entry AC Huffman tables. We must do the same here.
    init_chroma_ac_tables();
    size_t max_stream_bytes = (size_t)width * (size_t)height * 4; // generous
    UINT8 *bitstream = (UINT8*)malloc(max_stream_bytes);
    if (!bitstream) {
        printf("[SW TWIN] ERROR: cannot allocate bitstream buffer.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        free(Y_q); free(Cb_q); free(Cr_q);
        return -1;
    }

    size_t bitstream_len = encode_bitstream(Y_q, Cb_q, Cr_q,
                                            total_mcus,
                                            bitstream, max_stream_bytes);
    printf("[SW TWIN] Encoded bitstream length: %zu bytes\n", bitstream_len);

    // Build dynamic header
    UINT8 header[623];
    int header_size = build_dynamic_header(width, height,
                                           header, (int)sizeof(header));
    if (header_size < 0) {
        printf("[SW TWIN] ERROR: failed to build header.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        free(Y_q); free(Cb_q); free(Cr_q);
        free(bitstream);
        return -1;
    }

    // Patch DQT tables inside header to match the selected quality.
    // Header expects DQT tables in zig-zag order, while q_luma16/q_chroma16 are row-major.
    UINT8 luma_zz[64];
    UINT8 chroma_zz[64];
    for (i = 0; i < 64; i++) {
        UINT16 pos = zigzag_table[i];
        luma_zz[pos] = (UINT8)(q_luma16[i] & 0xFF);
        chroma_zz[pos] = (UINT8)(q_chroma16[i] & 0xFF);
    }
    if (patch_dqt_tables_in_header(header, header_size, luma_zz, chroma_zz) != 0) {
        printf("[SW TWIN] ERROR: could not locate DQT markers in header template.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        free(Y_q); free(Cb_q); free(Cr_q);
        free(bitstream);
        return -1;
    }

    // Write final JPEG file
    printf("[SW TWIN] Writing output JPEG: %s\n", out_path);
    FILE *f_out = fopen(out_path, "wb");
    if (!f_out) {
        printf("[SW TWIN] ERROR: cannot open output file.\n");
        free(rgb_buffer);
        free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
        free(Y_q); free(Cb_q); free(Cr_q);
        free(bitstream);
        return -1;
    }

    fwrite(header, 1, header_size, f_out);
    fwrite(bitstream, 1, bitstream_len, f_out);
    fclose(f_out);

    printf("[SW TWIN] Done. Output JPEG size: %d + %zu bytes\n",
           header_size, bitstream_len);

    free(rgb_buffer);
    free(Y_blocks); free(Cb_blocks); free(Cr_blocks);
    free(Y_q); free(Cb_q); free(Cr_q);
    free(bitstream);

    return 0;
}

