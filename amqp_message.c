#include "amqp_message.h"

#include <stdlib.h>
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
    return 0;
}

void unparse_message_header(amqp_message_header header, char *s) {
    header.channel = htons(header.channel);
    header.length = htonl(header.length);

    memcpy(s, &header, sizeof(amqp_message_header));
}

int parse_method_header(char *s, ssize_t n, amqp_method_header *header) {
    ssize_t header_size = sizeof(amqp_method_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);

    header->class = ntohs(header->class);
    header->method = ntohs(header->method);
    return 0;
}

void unparse_method_header(amqp_method_header header, char *s) {
    header.class = htons(header.class);
    header.method = htons(header.method);

    memcpy(s, &header, sizeof(amqp_method_header));
}

amqp_method *parse_method(char *s, ssize_t n) {
    ssize_t header_size = sizeof(amqp_method_header);
    amqp_method *method;

    if (n < header_size) return NULL;

    method = malloc(n);

    memcpy(method, s, header_size);

    method->header.class = ntohs(method->header.class);
    method->header.method = ntohs(method->header.method);

    return method;
}

amqp_content_header *parse_content_header(char *s, ssize_t n) {
    ssize_t header_size = sizeof(amqp_content_header_header);
    amqp_content_header *content_header;

    if (n < header_size) return NULL;

    content_header = malloc(n);

    memcpy(content_header, s, header_size);

    content_header->header.class = ntohs(content_header->header.class);
    content_header->header.weight = ntohs(content_header->header.weight);
    content_header->header.body_size = ntohll(content_header->header.body_size);

    return content_header;
}

int prepare_message(
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size,
    char *dest
    ) {

    size_t message_header_size = sizeof(amqp_message_header);
    size_t method_header_size = sizeof(amqp_method_header);
    size_t header_size = message_header_size + method_header_size;

    char unparsed[128];

    amqp_message_header message_header = {
        .msg_type = METHOD,
        .channel = channel,
        .length = args_size + 4
    };

    amqp_method_header method_header = {
        .class = class,
        .method = method
    };

    unparse_message_header(message_header, unparsed);
    unparse_method_header(method_header, unparsed + message_header_size);

    memcpy(dest, unparsed, header_size);
    memcpy(dest + header_size, arguments, args_size);

    dest[header_size + args_size] = 0xce;
    return header_size + args_size + 1;
}
