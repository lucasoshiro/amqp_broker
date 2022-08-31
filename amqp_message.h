#pragma once

#include <stdint.h>
#include <string.h>

#include "amqp_methods.h"
#include "amqp_types.h"

typedef struct {
    char amqp[4];
    uint8_t id_major;
    uint8_t id_minor;
    uint8_t version_major;
    uint8_t version_minor;
} __attribute__((packed)) amqp_protocol_header;

typedef struct {
    uint8_t msg_type;
    uint16_t channel;
    uint32_t length;
} __attribute__((packed)) amqp_message_header;


typedef struct {
    class_id class;
    method_id method;
} __attribute__((packed)) amqp_method_header;


int parse_protocol_header(char *s, ssize_t n, amqp_protocol_header *header);

int parse_message_header(char *s, ssize_t n, amqp_message_header *header);

void unparse_message_header(amqp_message_header header, char *s);

void print_message_header(amqp_message_header header);

int parse_method_header(char *s, ssize_t n, amqp_method_header *header);

void unparse_method_header(amqp_method_header header, char *s);

void print_method_header(amqp_method_header header);

int prepare_message(
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size,
    char *dest
    );
