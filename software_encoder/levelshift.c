#include "datatype.h"

// Subtracts 128 from each of the 64 elements in the block
void levelshift_sw(INT16 *block) {
    int i;
    for (i = 0; i < 64; i++) {
        block[i] -= 128;
    }
}