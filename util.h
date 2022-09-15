/* Util

These are miscellaneous functions.
*/
#pragma once

#include <stdint.h>
#include <stdlib.h>

/* Count the number of set bits in an 16 bit array. */
int bit_cardinality_16(uint16_t bit_array);

/* Make 64-bit little-endian numbers become big-endian and vice-versa.

   Even though some Unices provides a htonll and ntohll that can do this, this
   is not POSIX and they are not provided by some libcs, like glibc or musl.
   Then we need this function to make it.
 */
uint64_t swipe_endianness_64(uint64_t n);

/* Read from the file descriptor fildes into buf until it have the specified
   size. This function returns the size itself if everything is ok, or 0
   if there's no more bytes to read.
*/
size_t read_until(int fildes, void *buf, size_t size);
