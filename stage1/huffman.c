#include "nios2_common/datatype.h"
#include "huffdata.h"

// Global state for bit packing
UINT32 bit_buffer = 0;
int bit_count = 0;
UINT8 byte_buffer = 0;
int has_odd_byte = 0;

// Calculates the bit-size (category) of a value for JPEG VLI encoding
static int get_vli_size(INT16 val) {
    if (val == 0) return 0;
    if (val < 0) val = -val; // absolute value
    int size = 0;
    while (val > 0) {
        size++;
        val >>= 1;
    }
    return size;
}

// Writes bits, packs them into bytes, applies 0xFF byte stuffing,
// and bundles them into 16-bit words for the hardware FIFO.
int write_bits(UINT16 code, int size, INT16 *out_buffer) {
    int words_written = 0;
    if (size == 0) return 0;

    // Push new bits into the buffer
    bit_buffer = (bit_buffer << size) | (code & ((1 << size) - 1));
    bit_count += size;

    // Extract complete 8-bit bytes whenever we have enough bits
    while (bit_count >= 8) {
        bit_count -= 8;
        UINT8 out_byte = (bit_buffer >> bit_count) & 0xFF;

        if (!has_odd_byte) {
            byte_buffer = out_byte;
            has_odd_byte = 1;
        } else {
            // Pair the previous byte and current byte into a 16-bit word
            out_buffer[words_written++] = (INT16)((byte_buffer << 8) | out_byte);
            has_odd_byte = 0;
        }

        // JPEG Standard: If we emit 0xFF, the VERY NEXT byte MUST be 0x00
        if (out_byte == 0xFF) {
            if (!has_odd_byte) {
                byte_buffer = 0x00;
                has_odd_byte = 1;
            } else {
                out_buffer[words_written++] = (INT16)((byte_buffer << 8) | 0x00);
                has_odd_byte = 0;
            }
        }
    }
    return words_written;
}

// Flushes any remaining bits in the buffer at the end of the image
int huffman_flush(INT16 *out_buffer) {
    int words_written = 0;

    // If there are leftover bits, pad the rest of the byte with 1s
    if (bit_count > 0) {
        int pad_size = 8 - bit_count;
        words_written += write_bits((1 << pad_size) - 1, pad_size, out_buffer + words_written);
    }
    
    // If we have an odd byte waiting to be paired into a 16-bit word, pad it with 0xFF
    if (has_odd_byte) {
        out_buffer[words_written++] = (INT16)((byte_buffer << 8) | 0xFF);
        has_odd_byte = 0;
    }

    return words_written;
}

// The core Huffman compressor
int huffman_encode_block(INT16 *block, INT16 *prev_dc, int is_chroma, INT16 *out_fifo_buffer) {
    int words_generated = 0;
    
    // Select the correct tables based on Luminance vs. Chrominance
    UINT16 *dc_codes = is_chroma ? chrominance_dc_code_table : luminance_dc_code_table;
    UINT16 *dc_sizes = is_chroma ? chrominance_dc_size_table : luminance_dc_size_table;
    UINT16 *ac_codes = is_chroma ? chrominance_ac_code_table : luminance_ac_code_table;
    UINT16 *ac_sizes = is_chroma ? chrominance_ac_size_table : luminance_ac_size_table;

    // 1. DPCM Encode the DC Coefficient
    INT16 dc_diff = block[0] - *prev_dc;
    *prev_dc = block[0];
    
    int dc_size = get_vli_size(dc_diff);
    // Standard JPEG 1s complement for negative values
    UINT16 dc_vli = (dc_diff < 0) ? (dc_diff - 1) & ((1 << dc_size) - 1) : dc_diff;

    words_generated += write_bits(dc_codes[dc_size], dc_sizes[dc_size], out_fifo_buffer + words_generated);
    if (dc_size > 0) {
        words_generated += write_bits(dc_vli, dc_size, out_fifo_buffer + words_generated);
    }
    
    // 2. Run-Length Encode the AC Coefficients
    int zero_run = 0;
    int i;
    for (i = 1; i < 64; i++) {
        if (block[i] == 0) {
            zero_run++;
        } else {
            // Write ZRL (Zero Run Length) markers for every 16 consecutive zeros
            while (zero_run >= 16) {
                words_generated += write_bits(ac_codes[0xF0], ac_sizes[0xF0], out_fifo_buffer + words_generated);
                zero_run -= 16;
            }

            int ac_size = get_vli_size(block[i]);
            UINT16 ac_vli = (block[i] < 0) ? (block[i] - 1) & ((1 << ac_size) - 1) : block[i];

            // The high nibble is the run length, the low nibble is the size
            int symbol = (zero_run << 4) | ac_size;

            words_generated += write_bits(ac_codes[symbol], ac_sizes[symbol], out_fifo_buffer + words_generated);
            words_generated += write_bits(ac_vli, ac_size, out_fifo_buffer + words_generated);
            zero_run = 0;
        }
    }
    
    // 3. Write End of Block (EOB) if block ends with zeros
    if (zero_run > 0) {
        words_generated += write_bits(ac_codes[0x00], ac_sizes[0x00], out_fifo_buffer + words_generated);
    }
    
    return words_generated;
}
