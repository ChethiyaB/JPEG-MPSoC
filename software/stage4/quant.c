#include "nios2_common/datatype.h"
#include "quantdata.h"

// Standard JPEG Base Quantization Tables
static const UINT8 std_lum_quant[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68,109,103, 77,
    24, 35, 55, 64, 81,104,113, 92,
    49, 64, 78, 87,103,121,120,101,
    72, 92, 95, 98,112,100,103, 99
};

static const UINT8 std_chr_quant[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

// Generates scaled tables based on the requested quality (1-100)
void initialize_quantization_tables(UINT32 quality, UINT16 *Lqt, UINT16 *Cqt) {
    int i;
    UINT32 scale_factor;

    if (quality <= 0) quality = 1;
    if (quality > 100) quality = 100;
    
    if (quality < 50) scale_factor = 5000 / quality;
    else scale_factor = 200 - quality * 2;

    for (i = 0; i < 64; i++) {
        // Luminance
        UINT32 temp = (std_lum_quant[i] * scale_factor + 50) / 100;
        if (temp < 1) temp = 1;
        if (temp > 255) temp = 255;
        // Notice we do NOT zig-zag the table here yet, just scale it.
        // The zig-zag happens during the actual quantization pass.
        Lqt[i] = temp;

        // Chrominance
        temp = (std_chr_quant[i] * scale_factor + 50) / 100;
        if (temp < 1) temp = 1;
        if (temp > 255) temp = 255;
        Cqt[i] = temp;
    }
}

// Applies quantization and writes output in zig-zag order
void quantization(INT16 *in, UINT16 *quant_table, INT16 *out) {
    int i;
    for (i = 0; i < 64; i++) {
        INT32 coeff = in[i];
        // Fast integer division with rounding
        INT16 q_val = (INT16)((coeff > 0) ? (coeff + (quant_table[i] >> 1)) / quant_table[i] 
                                          : (coeff - (quant_table[i] >> 1)) / quant_table[i]);
        // Write to output using the zig-zag lookup table
        out[zigzag_table[i]] = q_val;
    }
}
