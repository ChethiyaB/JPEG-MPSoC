#ifndef ENCODER_H
#define ENCODER_H

#include "../nios2_common/datatype.h"

// Base address for the FIFO connecting Stage 1 to Stage 2
// (In Quartus/Eclipse, this will come from system.h. We define it here for testing)
#ifndef FIFO_OUT_BASE
#define FIFO_OUT_BASE     0x00000000 
#define FIFO_OUT_CSR_BASE 0x00000000
#endif

void encode_image_stage1(UINT8 *rgb_in, int width, int height);

#endif