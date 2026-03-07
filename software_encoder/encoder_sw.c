#include "encoder_sw.h"
#include "jdatatype.h"
#include "quantdata.h"
#include "huffman_sw.h"

// Defined in previous steps
extern void read_444_format_sw(JPEG_ENCODER_STRUCTURE *jpeg, UINT8 *input, INT16 *Y, INT16 *CB, INT16 *CR, int mcu_col, int mcu_row);
extern void levelshift_sw(INT16 *block);
extern void dct_sw(INT16 *data);
extern void initialize_quantization_tables_sw(UINT32 quality, UINT16 *Lqt, UINT16 *Cqt, UINT16 *ILqt, UINT16 *ICqt);
extern void quantization_sw(INT16 *in, UINT16 *quant_table, INT16 *out);

int encode_image_sw(UINT8 *rgb_in, UINT8 *jpeg_out, int width, int height) {
    JPEG_ENCODER_STRUCTURE jpeg;
    jpeg.cols = width;
    jpeg.rows = height;
    
    // In 4:4:4, 1 MCU = 8x8 pixels.
    int mcus_x = (width + 7) / 8;
    int mcus_y = (height + 7) / 8;
    
    UINT16 Lqt[64], Cqt[64], ILqt[64], ICqt[64];
    initialize_quantization_tables_sw(90, Lqt, Cqt, ILqt, ICqt); // Quality 90
    
    huffman_init_sw(jpeg_out);
    write_markers_sw(width, height);
    
    INT16 Y_in[64], Cb_in[64], Cr_in[64];
    INT16 Y_out[64], Cb_out[64], Cr_out[64];
    
    int row, col;
    for (row = 0; row < mcus_y; row++) {
        for (col = 0; col < mcus_x; col++) {
            // STAGE 1: Read and Convert
            read_444_format_sw(&jpeg, rgb_in, Y_in, Cb_in, Cr_in, col, row);
            
            // STAGE 2: Level Shift
            levelshift_sw(Y_in); levelshift_sw(Cb_in); levelshift_sw(Cr_in);
            
            // STAGE 3: DCT
            dct_sw(Y_in); dct_sw(Cb_in); dct_sw(Cr_in);
            
            // STAGE 4: Quantization
            quantization_sw(Y_in, Lqt, Y_out);
            quantization_sw(Cb_in, Cqt, Cb_out);
            quantization_sw(Cr_in, Cqt, Cr_out);
            
            // STAGE 5 & 6: Huffman & Bitstream Write
            huffman_encode_block_sw(Y_out, 0);  // 0 = Luma
            huffman_encode_block_sw(Cb_out, 1); // 1 = Chroma
            huffman_encode_block_sw(Cr_out, 2); // 2 = Chroma (using same tables but diff DC state)
        }
    }
    
    return huffman_finish_sw();
}