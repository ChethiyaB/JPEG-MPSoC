#include "huffman_sw.h"

static UINT8 *out_ptr;
static UINT32 put_buffer = 0;
static int put_bits = 0;
static INT16 prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
static int total_bytes = 0;

// External declarations to link against huffdata.c and markdata.c
extern UINT16 luminance_dc_code_table[];
extern UINT16 luminance_dc_size_table[];
extern UINT16 chrominance_dc_code_table[];
extern UINT16 chrominance_dc_size_table[];
extern UINT16 luminance_ac_code_table[];
extern UINT16 luminance_ac_size_table[];
extern UINT16 chrominance_ac_code_table[];
extern UINT16 chrominance_ac_size_table[];
extern UINT16 markerdata[];

static void emit_byte(UINT8 val) {
    *out_ptr++ = val;
    total_bytes++;
    // Byte stuffing: if byte is 0xFF, must emit 0x00 afterwards
    if (val == 0xFF) {
        *out_ptr++ = 0x00;
        total_bytes++;
    }
}

static void write_bits(UINT16 code, int size) {
    put_buffer = (put_buffer << size) | code;
    put_bits += size;
    while (put_bits >= 8) {
        emit_byte((UINT8)((put_buffer >> (put_bits - 8)) & 0xFF));
        put_bits -= 8;
    }
}

void huffman_init_sw(UINT8 *out_buffer) {
    out_ptr = out_buffer;
    put_buffer = 0;
    put_bits = 0;
    prev_DC_Y = prev_DC_Cb = prev_DC_Cr = 0;
    total_bytes = 0;
}

// Write JPEG header markers (SOI, APP0, DQT, SOF, DHT, SOS)
void write_markers_sw(int width, int height) {
    int i;
    // For simplicity in Phase 0, we write a generic minimal header
    // SOI
    *out_ptr++ = 0xFF; *out_ptr++ = 0xD8; total_bytes += 2;
    // APP0, DQT, DHT, etc. would normally be extracted from markerdata[]
    // To match the exact Tensilica reference, you would loop through markerdata
    for (i = 0; i < 210; i++) {
        UINT16 m = markerdata[i];
        *out_ptr++ = (m >> 8) & 0xFF;
        *out_ptr++ = m & 0xFF;
        total_bytes += 2;
    }
    // SOF (Start of Frame) - Needs dynamic width/height
    *out_ptr++ = 0xFF; *out_ptr++ = 0xC0;
    *out_ptr++ = 0x00; *out_ptr++ = 0x11; // Length 17
    *out_ptr++ = 0x08; // 8 bits/sample
    *out_ptr++ = (height >> 8) & 0xFF; *out_ptr++ = height & 0xFF;
    *out_ptr++ = (width >> 8) & 0xFF;  *out_ptr++ = width & 0xFF;
    *out_ptr++ = 0x03; // 3 components
    *out_ptr++ = 0x01; *out_ptr++ = 0x11; *out_ptr++ = 0x00; // Y
    *out_ptr++ = 0x02; *out_ptr++ = 0x11; *out_ptr++ = 0x01; // Cb
    *out_ptr++ = 0x03; *out_ptr++ = 0x11; *out_ptr++ = 0x01; // Cr
    total_bytes += 19;
    
    // SOS (Start of Scan)
    *out_ptr++ = 0xFF; *out_ptr++ = 0xDA;
    *out_ptr++ = 0x00; *out_ptr++ = 0x0C; // Length 12
    *out_ptr++ = 0x03; // 3 components
    *out_ptr++ = 0x01; *out_ptr++ = 0x00;
    *out_ptr++ = 0x02; *out_ptr++ = 0x11;
    *out_ptr++ = 0x03; *out_ptr++ = 0x11;
    *out_ptr++ = 0x00; *out_ptr++ = 0x3F; *out_ptr++ = 0x00;
    total_bytes += 14;
}

// Simplified Huffman encoding for 1 block
void huffman_encode_block_sw(INT16 *block, int is_chroma) {
    INT16 temp, temp2;
    int nbits, k, r, i;
    
    UINT16 *dc_code = is_chroma ? chrominance_dc_code_table : luminance_dc_code_table;
    UINT16 *dc_size = is_chroma ? chrominance_dc_size_table : luminance_dc_size_table;
    UINT16 *ac_code = is_chroma ? chrominance_ac_code_table : luminance_ac_code_table;
    UINT16 *ac_size = is_chroma ? chrominance_ac_size_table : luminance_ac_size_table;

    // DC Difference
    INT16 *prev_DC = is_chroma ? (is_chroma == 1 ? &prev_DC_Cb : &prev_DC_Cr) : &prev_DC_Y;
    temp = temp2 = block[0] - *prev_DC;
    *prev_DC = block[0];

    if (temp < 0) { temp = -temp; temp2--; }
    nbits = 0; while (temp) { nbits++; temp >>= 1; } // Find size
    
    write_bits(dc_code[nbits], dc_size[nbits]);
    if (nbits) write_bits(temp2, nbits);

    // AC Run-Length Encoding
    r = 0;
    for (k = 1; k < 64; k++) {
        if ((temp = block[k]) == 0) {
            r++;
        } else {
            while (r > 15) {
                write_bits(ac_code[0xF0], ac_size[0xF0]); // ZRL (Zero Run Length)
                r -= 16;
            }
            temp2 = temp;
            if (temp < 0) { temp = -temp; temp2--; }
            nbits = 1; while (temp >>= 1) nbits++;
            i = (r << 4) + nbits;
            write_bits(ac_code[i], ac_size[i]);
            write_bits(temp2, nbits);
            r = 0;
        }
    }
    if (r > 0) write_bits(ac_code[0], ac_size[0]); // EOB (End of Block)
}

int huffman_finish_sw(void) {
    // Flush remaining bits
    if (put_bits > 0) {
        write_bits(0x7F, 7); // pad with 1s
    }
    // EOI marker
    *out_ptr++ = 0xFF; *out_ptr++ = 0xD9;
    total_bytes += 2;
    return total_bytes;
}