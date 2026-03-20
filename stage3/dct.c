#include <math.h>
#include "nios2_common/datatype.h"

// 1. Precomputed Look-Up Tables (LUTs)
static float cos_lut[8][8];
static float alpha_lut[8];
static int lut_initialized = 0;

// Calculate the heavy math exactly ONCE
void init_dct_lut() {
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            // Note: We use 'float' casting to avoid heavy 'double' precision math
            cos_lut[i][j] = (float)cos((2.0 * i + 1.0) * j * 3.14159265358979323846 / 16.0);
        }
    }

    // Precompute alpha values to avoid sqrt() in the loop
    alpha_lut[0] = 1.0f / (float)sqrt(2.0);
    for (i = 1; i < 8; i++) {
        alpha_lut[i] = 1.0f;
    }

    lut_initialized = 1;
}

void dct(INT16 *block) {
    // Ensure tables are built on the very first run
    if (!lut_initialized) {
        init_dct_lut();
    }

    float temp[8][8];
    int u, v, x, y;

    // Calculate DCT using Look-Up Tables
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            float sum = 0.0f;
            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    // FAST: Array memory reads instead of 24,000 cos() calls!
                    sum += (float)block[x * 8 + y] * cos_lut[x][u] * cos_lut[y][v];
                }
            }
            temp[u][v] = 0.25f * alpha_lut[u] * alpha_lut[v] * sum;
        }
    }

    // Copy the transformed frequency coefficients back
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            // roundf() is faster than double-precision round()
            block[u * 8 + v] = (INT16)roundf(temp[u][v]);
        }
    }
}
