#include <math.h>
#include "../nios2_common/datatype.h"

#define PI 3.14159265358979323846

void dct(INT16 *block) {
    float temp[8][8];
    float alpha_u, alpha_v;
    int u, v, x, y;

    // Calculate DCT
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            float sum = 0.0;
            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    sum += block[x * 8 + y] * cos((2.0 * x + 1.0) * u * PI / 16.0) * cos((2.0 * y + 1.0) * v * PI / 16.0);
                }
            }
            alpha_u = (u == 0) ? (1.0 / sqrt(2.0)) : 1.0;
            alpha_v = (v == 0) ? (1.0 / sqrt(2.0)) : 1.0;
            temp[u][v] = 0.25 * alpha_u * alpha_v * sum;
        }
    }

    // Copy the transformed frequency coefficients back to the block array
    for (u = 0; u < 8; u++) {
        for (v = 0; v < 8; v++) {
            block[u * 8 + v] = (INT16)round(temp[u][v]);
        }
    }
}