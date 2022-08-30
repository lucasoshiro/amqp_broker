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

void unparse_message_header(amqp_message_header header, char *s) {
    header.channel = htons(header.channel);
    header.length = htonl(header.length);
    header.class = htons(header.class);
    header.method = htons(header.method);

    memcpy(s, &header, sizeof(amqp_message_header));
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

int prepare_message(
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size,
    char *dest
    ) {

    size_t header_size = sizeof(amqp_message_header);
    char unparsed[128];

    amqp_message_header header = {
        .msg_type = METHOD,
        .channel = channel,
        .length = args_size + 4,
        .class = class,
        .method = method
    };

    unparse_message_header(header, unparsed);    

    memcpy(dest, unparsed, header_size);
    memcpy(dest + header_size, arguments, args_size);

    dest[header_size + args_size] = 0xce;
    return header_size + args_size + 1;
}
