#include "datatype.h"

#define c1 1420
#define c2 1338
#define c3 1204
#define c5 805
#define c6 554
#define c7 283
#define s1 3
#define s2 10
#define s3 13

// 1D DCT pass used for both rows and columns
static void dct_1d(INT16 *data, int stride) {
    int i;
    for (i = 0; i < 8; i++) {
        INT16 *ptr = data + (i * stride);
        
        // standard integer DCT butterfly network using provided constants
        INT32 x0 = ptr[0] + ptr[7]; INT32 x7 = ptr[0] - ptr[7];
        INT32 x1 = ptr[1] + ptr[6]; INT32 x6 = ptr[1] - ptr[6];
        INT32 x2 = ptr[2] + ptr[5]; INT32 x5 = ptr[2] - ptr[5];
        INT32 x3 = ptr[3] + ptr[4]; INT32 x4 = ptr[3] - ptr[4];

        INT32 x8 = x0 + x3; INT32 x11 = x0 - x3;
        INT32 x9 = x1 + x2; INT32 x10 = x1 - x2;

        ptr[0] = (x8 + x9) << s1;
        ptr[4] = (x8 - x9) << s1;

        INT32 w1 = (x10 + x11) * c6;
        ptr[2] = (w1 + x11 * (c2 - c6)) >> s2;
        ptr[6] = (w1 - x10 * (c2 + c6)) >> s2;

        INT32 z1 = (x4 + x7) * c7;
        INT32 z2 = (x5 + x6) * c5;
        INT32 z3 = (x4 + x6) * c3;
        INT32 z4 = (x5 + x7) * c1;

        ptr[1] = (z4 + z3 + z1 - x7 * (c1 + c7 - c3)) >> s3;
        ptr[3] = (z4 - z2 - z1 - x5 * (c1 - c5 + c7)) >> s3;
        ptr[5] = (z2 + z3 - z4 - x6 * (c5 + c3 - c1)) >> s3;
        ptr[7] = (z2 + z1 - z3 - x4 * (c5 + c7 - c3)) >> s3;
    }
}

// In-place 2D DCT: row DCT followed by column DCT
void dct_sw(INT16 *data) {
    // Row pass (stride 1, but we do it block-wise so 8 blocks of 1)
    // For simplicity, we implement it as sequential 8-element chunks
    int i;
    for (i = 0; i < 8; i++) {
        dct_1d(data + i * 8, 1);
    }
    // Column pass (stride 8)
    dct_1d(data, 8);
}