#include "nios2_common/datatype.h"
#include "nios2_common/jdatatype.h"

static inline INT16 clamp(INT16 val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

void read_444_format(UINT8 *input_ptr, int width, int height, INT16 *Y1, INT16 *CB, INT16 *CR, int mcu_col, int mcu_row) {
    int i, j;
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            int img_x = mcu_col * 8 + j;
            int img_y = mcu_row * 8 + i;
            if (img_x >= width) img_x = width - 1;
            if (img_y >= height) img_y = height - 1;
            
            int pixel_idx = (img_y * width + img_x) * 3;
            INT16 r = input_ptr[pixel_idx];
            INT16 g = input_ptr[pixel_idx + 1];
            INT16 b = input_ptr[pixel_idx + 2];
            
            // RGB to YCbCr
            INT16 y  = (( 77 * r + 150 * g +  29 * b) >> 8);
            INT16 cb = ((-43 * r -  85 * g + 128 * b) >> 8) + 128;
            INT16 cr = ((128 * r - 107 * g -  21 * b) >> 8) + 128;
            
            int block_idx = i * 8 + j;
            Y1[block_idx] = clamp(y);
            CB[block_idx] = clamp(cb);
            CR[block_idx] = clamp(cr);
        }
    }
}
