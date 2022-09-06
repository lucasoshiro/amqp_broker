#include "util.h"

#include <arpa/inet.h>

int bit_cardinality_16(uint16_t bit_array) {
    int c = 0;

    for (int i = 0; i < 16; i++) {
        c += bit_array & 0x1;
        bit_array >>= 1;
    }

    return c;
}

uint64_t swipe_endianness_64(uint64_t n) {
    uint32_t left = (uint32_t) (n >> 32);
    uint32_t right = (uint32_t) (n & 0xffff);    

    uint32_t new_left = htonl(right);
    uint32_t new_right = htonl(left);

    return ((uint64_t) new_left << 32) | ((uint64_t) new_right);
}
