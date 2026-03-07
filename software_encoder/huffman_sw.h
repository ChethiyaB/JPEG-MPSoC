#ifndef HUFFMAN_SW_H
#define HUFFMAN_SW_H

#include "datatype.h"

void huffman_init_sw(UINT8 *out_buffer);
void write_markers_sw(int width, int height);
void huffman_encode_block_sw(INT16 *block, int is_chroma);
int huffman_finish_sw(void);

#endif // HUFFMAN_SW_H