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

typedef struct {
    amqp_method_header header;
    char arguments[1];
} __attribute__((packed)) amqp_method;

typedef struct {
    uint16_t class;
    uint16_t weight;
    uint64_t body_size;
    uint16_t property_flags;
} __attribute__((packed)) amqp_content_header_header;

typedef struct {
    amqp_content_header_header header;
    char property_list[1];
} __attribute__((packed)) amqp_content_header;

int parse_protocol_header(char *s, ssize_t n, amqp_protocol_header *header);

int parse_message_header(char *s, ssize_t n, amqp_message_header *header);
void unparse_message_header(amqp_message_header header, char *s);

int parse_method_header(char *s, ssize_t n, amqp_method_header *header);
void unparse_method_header(amqp_method_header header, char *s);

void unparse_content_header_header(
    amqp_content_header_header header,
    char *s
    );
amqp_method *parse_method(char *s, ssize_t n);

amqp_content_header *parse_content_header(char *s, ssize_t n);

int prepare_message(
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size,
    char *dest
    );

int prepare_content_header(
    uint16_t class,
    uint16_t channel,
    uint16_t weight,
    uint64_t body_size,
    uint16_t flags,
    char *properties,
    char *dest
    );

int prepare_content_body(
    int channel,
    char *payload,
    size_t n,
    char *dest
    );
