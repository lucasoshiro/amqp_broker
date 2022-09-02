#include "util.h"

int bit_cardinality_16(uint16_t bit_array) {
    int c = 0;

    for (int i = 0; i < 16; i++) {
        c += bit_array & 0x1;
        bit_array >>= 1;
    }

    return c;
}
