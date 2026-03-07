#include "../nios2_common/datatype.h"

void levelshift(INT16 *block) {
    int i;
    for (i = 0; i < 64; i++) {
        block[i] -= 128;
    }
}