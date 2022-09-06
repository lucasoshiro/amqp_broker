#pragma once

#include <stdint.h>
#include <string.h>

#include "amqp_methods.h"
#include "connection_state.h"

typedef uint16_t class_id;
typedef uint16_t method_id;

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

int read_protocol_header(connection_state *cs, amqp_protocol_header *header);

int read_message_header(connection_state *cs, amqp_message_header *header);

amqp_method *read_method(connection_state *cs, int length);

amqp_content_header *read_content_header(connection_state *cs, int length);

char *read_body(connection_state *cs, int length);

void send_method(
    connection_state *cs,
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size
    );

void send_content_header(
    connection_state *cs,
    uint16_t class,
    uint16_t channel,
    uint16_t weight,
    uint64_t body_size,
    uint16_t flags,
    char *properties
    );

void send_body(
    connection_state *cs,
    int channel,
    char *payload,
    size_t n
    );
