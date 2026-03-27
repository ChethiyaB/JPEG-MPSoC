#include "system.h"
#include "nios2_common/datatype.h"

// Check your system.h for the exact macro name Altera generated!
#define CMD_WRITE   0
#define CMD_COMPUTE 1
#define CMD_READ    2

void dct(INT16 *block) {
    int i;

    // 1. Write the 64 pixels into the Hardware Accelerator
    for (i = 0; i < 64; i++) {
        // Macro parameters: (n_value, dataa, datab)
    	ALT_CI_DCT_ACCELERATOR(CMD_WRITE, i, block[i]);
    }

    // 2. Fire the Hardware Compute!
    // The CPU will freeze on this exact line until the Verilog asserts 'done'
    ALT_CI_DCT_ACCELERATOR(CMD_COMPUTE, 0, 0);

    // 3. Read the 64 transformed coefficients back into the C array
    for (i = 0; i < 64; i++) {
        block[i] = (INT16)ALT_CI_DCT_ACCELERATOR(CMD_READ, i, 0);
    }
}
