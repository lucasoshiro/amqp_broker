#pragma once

#include <stdint.h>
#include <string.h>

typedef struct {
    char amqp[4];
    uint8_t id_major;
    uint8_t id_minor;
    uint8_t version_major;
    uint8_t version_minor;
} amqp_header;

int parse_header(char *s, ssize_t n, amqp_header *header);
