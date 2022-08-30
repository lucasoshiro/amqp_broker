#include "amqp_message.h"

#include <stdio.h>
#include <arpa/inet.h>

const amqp_protocol_header default_amqp_header = {
    .amqp = {'A', 'M', 'Q', 'P'},
    .id_major = 0,
    .id_minor = 0,
    .version_major = 9,
    .version_minor = 1
};

int parse_protocol_header(char *s, ssize_t n, amqp_protocol_header *header) {
    ssize_t header_size = sizeof(amqp_protocol_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);
    return memcmp(header, &default_amqp_header, header_size);
}

int parse_message_header(char *s, ssize_t n, amqp_message_header *header) {
    ssize_t header_size = sizeof(amqp_message_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);

    header->channel = ntohs(header->channel);
    header->length = ntohl(header->length);
    header->class = ntohs(header->class);
    header->method = ntohs(header->method);
    return 0;
}

void print_message_header(amqp_message_header header) {
    printf(
        "type: %02x\n"
        "channel: %04x\n"
        "length: %08x\n"
        "l2: %x\n"
        "class: %04x\n"
        "method: %04x\n",
        header.msg_type, header.channel, header.length, ntohl(header.length),
        header.class, header.method
        );
}
