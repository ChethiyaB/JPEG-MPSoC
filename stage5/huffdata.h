#ifndef HUFFDATA_H
#define HUFFDATA_H

#include "nios2_common/datatype.h"

// MUST be UINT16 to prevent hardware FIFO memory overflows!
extern UINT16 luminance_dc_code_table[12];
extern UINT16 luminance_dc_size_table[12];
extern UINT16 chrominance_dc_code_table[12];
extern UINT16 chrominance_dc_size_table[12];

extern UINT16 luminance_ac_code_table[256];
extern UINT16 luminance_ac_size_table[256];
extern UINT16 chrominance_ac_code_table[256];
extern UINT16 chrominance_ac_size_table[256];

void init_chroma_ac_tables();

#endif
