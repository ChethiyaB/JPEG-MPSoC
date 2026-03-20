#ifndef ENCODER_H
#define ENCODER_H

#include "nios2_common/datatype.h"
#include <stdio.h>

// Encodes a single 8-pixel high strip of MCUs across the width of the image
void encode_mcu_row(UINT8 *strip_buffer, int width, int mcus_x, unsigned int fifo_base, unsigned int fifo_csr, FILE *f_stage1);

#endif
