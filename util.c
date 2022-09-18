#include "util.h"

#include <arpa/inet.h>
#include <unistd.h>

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

size_t read_until(int fildes, void *buf, size_t size) {
    size_t sum = 0;
    char *_buf = buf;

    while (sum < size) {
        size_t remaining = size - sum;
        size_t n = read(fildes, _buf, remaining);

        if (n == 0) return 0;

        sum += n;
        _buf += n;
    }
    return sum;
}
