#include "nios2_common/datatype.h"
#include <math.h>

// 1. Precomputed Look-Up Tables (LUTs) in Fixed-Point (Q8 precision)
// Scale factor = 256 (which is 1 << 8)
static int C_lut[8][8];
static int lut_initialized = 0;

void init_dct_lut() {
    int u, x;
    float alpha_u;

    // We use math.h ONLY ONCE during startup to generate the integer tables!
    for (u = 0; u < 8; u++) {
        if (u == 0) alpha_u = 1.0f / sqrtf(2.0f);
        else        alpha_u = 1.0f;

        for (x = 0; x < 8; x++) {
            // C(u,x) = 0.5 * alpha(u) * cos( (2x+1)*u*PI / 16 )
            float val = 0.5f * alpha_u * cosf((2.0f * x + 1.0f) * u * 3.14159265358979323846f / 16.0f);

            // Multiply by 256 to convert to Q8 integer format
            C_lut[u][x] = (int)(val * 256.0f);
        }
    }
    lut_initialized = 1;
}

void dct(INT16 *block) {
    if (!lut_initialized) {
        init_dct_lut();
    }

    int temp[8][8];
    int u, v, x, y;

    // --- PASS 1: 1D DCT ON ROWS ---
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            int sum = 0;
            for (x = 0; x < 8; x++) {
                // C_lut is Q8, block is Q0. sum accumulates as Q8.
                sum += C_lut[u][x] * block[x * 8 + v];
            }
            temp[u][v] = sum; 
        }
    }

    // --- PASS 2: 1D DCT ON COLUMNS ---
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            int sum = 0;
            for (y = 0; y < 8; y++) {
                // temp is Q8, C_lut is Q8. sum accumulates as Q16.
                sum += temp[u][y] * C_lut[v][y];
            }

            // Shift back from Q16 to Q0 integer space.
            // Adding (1 << 15) before shifting perfectly rounds the integer!
            block[u * 8 + v] = (INT16)((sum + 32768) >> 16);
        }
    }
}
