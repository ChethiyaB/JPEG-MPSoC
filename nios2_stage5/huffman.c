#include "../nios2_common/datatype.h"
#include "huffdata.h"

// Global state for bit packing
UINT32 bit_buffer = 0;
int bit_count = 0;
INT16 dc_y_prev = 0, dc_cb_prev = 0, dc_cr_prev = 0;

// Writes 16-bit words into our output array. Returns the number of words written.
int write_bits(UINT16 code, int size, INT16 *out_buffer, int out_idx) {
    int words_written = 0;
    bit_buffer = (bit_buffer << size) | (code & ((1 << size) - 1));
    bit_count += size;
    
    // When we have 16 or more bits, push them as a complete word
    while (bit_count >= 16) {
        bit_count -= 16;
        UINT16 out_word = (bit_buffer >> bit_count) & 0xFFFF;
        out_buffer[out_idx++] = (INT16)out_word;
        words_written++;
        
        // JPEG standard requires byte stuffing: if we output 0xFF, we must follow with 0x00
        if ((out_word >> 8) == 0xFF) {
            out_buffer[out_idx++] = 0x0000; // Simplified byte-stuffing logic for 16-bit word
            words_written++;
        }
    }
    return words_written;
}

// A simplified skeleton of the Huffman compression
int huffman_encode_block(INT16 *block, INT16 *prev_dc, int is_chroma, INT16 *out_fifo_buffer) {
    int words_generated = 0;
    
    // 1. Calculate DC difference
    INT16 dc_diff = block[0] - *prev_dc;
    *prev_dc = block[0];
    
    // (In full implementation: look up DC size and code based on dc_diff, then call write_bits)
    // words_generated += write_bits(dc_code, dc_size, out_fifo_buffer, words_generated);
    
    // 2. Encode AC coefficients with Run-Length Encoding
    int i;
    int zero_run = 0;
    for (i = 1; i < 64; i++) {
        if (block[i] == 0) {
            zero_run++;
        } else {
            while (zero_run >= 16) {
                // Write ZRL (Zero Run Length) marker for 16 consecutive zeros
                // words_generated += write_bits(ZRL_code, ZRL_size, ...);
                zero_run -= 16;
            }
            // Write the actual non-zero AC coefficient
            // words_generated += write_bits(ac_code, ac_size, ...);
            zero_run = 0;
        }
    }
    
    // 3. Write End of Block (EOB) if block ends with zeros
    if (zero_run > 0) {
        // words_generated += write_bits(EOB_code, EOB_size, ...);
    }
    
    return words_generated;
}