#include "amqp_message.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "log.h"

#define MAXLINE 4096

const amqp_protocol_header default_amqp_header = {
    .amqp = {'A', 'M', 'Q', 'P'},
    .id_major = 0,
    .id_minor = 0,
    .version_major = 9,
    .version_minor = 1
};

int parse_protocol_header(char *s, size_t n, amqp_protocol_header *header) {
    size_t header_size = sizeof(amqp_protocol_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);
    return memcmp(header, &default_amqp_header, header_size);
}

int read_protocol_header(connection_state *cs, amqp_protocol_header *header) {
    size_t n = read(cs->connfd, cs->recvline, sizeof(header));
    return parse_protocol_header(cs->recvline, n, header);
}

int parse_message_header(char *s, size_t n, amqp_message_header *header) {
    size_t header_size = sizeof(amqp_message_header);

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

int read_message_header(connection_state *cs, amqp_message_header *header) {
    size_t n;
    n = read(cs->connfd, cs->recvline, sizeof(*header));
    if (parse_message_header(cs->recvline, n, header)) return 1;
    log_message_header('C', *header);
    return 0;
}

int parse_method_header(char *s, size_t n, amqp_method_header *header) {
    size_t header_size = sizeof(amqp_method_header);

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

amqp_method *parse_method(char *s, size_t n) {
    size_t header_size = sizeof(amqp_method_header);
    amqp_method *method;

    if (n < header_size) return NULL;

    method = malloc(n);

    memcpy(method, s, header_size);

    method->header.class = ntohs(method->header.class);
    method->header.method = ntohs(method->header.method);

    return method;
}

amqp_method *read_method(connection_state *cs, int length) {
    size_t n;
    amqp_method *method;

    n = read(cs->connfd, cs->recvline, length);
    method = parse_method(cs->recvline, n);
    n = read(cs->connfd, cs->recvline, 1);

    return n == 0 ? NULL : method;
}

void unparse_content_header_header(
    amqp_content_header_header header,
    char *s
    ) {
    header.class = htons(header.class);
    header.weight = htons(header.weight);
    header.body_size = swipe_endianness_64(header.body_size);
    header.property_flags = htons(header.property_flags);

    memcpy(s, &header, sizeof(header));
}

amqp_content_header *parse_content_header(char *s, size_t n) {
    size_t header_size = sizeof(amqp_content_header_header);
    amqp_content_header *content_header;

    if (n < header_size) return NULL;

    content_header = malloc(n);

    memcpy(content_header, s, header_size);

    content_header->header.class = ntohs(content_header->header.class);
    content_header->header.weight = ntohs(content_header->header.weight);
    content_header->header.body_size = swipe_endianness_64(content_header->header.body_size);
    content_header->header.property_flags = ntohs(content_header->header.property_flags);

    return content_header;
}

amqp_content_header *read_content_header(connection_state *cs, int length) {
    size_t n;
    amqp_content_header *header;
    n = read(cs->connfd, cs->recvline, length);
    header = parse_content_header(cs->recvline, length);
    n = read(cs->connfd, cs->recvline, 1);

    return n == 0 ? NULL : header;
}

char *read_body(connection_state *cs, int length) {
    int n = read(cs->connfd, cs->recvline, length + 1);
    char *body = malloc(length + 1);
    memcpy(body, cs->recvline, n);
    body[length] = '\0';
    return body;
}

void send_method(
    connection_state *cs,
    class_id class,
    method_id method,
    uint16_t channel,
    void *arguments,
    size_t args_size
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

    memcpy(cs->sendline, unparsed, header_size);
    memcpy(cs->sendline + header_size, arguments, args_size);

    cs->sendline[header_size + args_size] = 0xce;
    write(cs->connfd, cs->sendline, header_size + args_size + 1);

    log_message_header('S', message_header);
}

void send_content_header(
    connection_state *cs,
    uint16_t class,
    uint16_t channel,
    uint16_t weight,
    uint64_t body_size,
    uint16_t flags,
    char *properties
    ) {

    size_t message_header_size = sizeof(amqp_message_header);
    size_t header_header_size = sizeof(amqp_content_header_header);
    size_t properties_size = bit_cardinality_16(flags);

    amqp_message_header message_header = {
        .msg_type = CONTENT_HEADER,
        .channel = channel,
        .length = header_header_size + properties_size
    };

    amqp_content_header_header header_header = {
        .class = class,
        .weight = weight,
        .body_size = body_size,
        .property_flags = flags
    };

    unparse_message_header(message_header, cs->sendline);
    unparse_content_header_header(
        header_header,
        cs->sendline + message_header_size
        );

    memcpy(cs->sendline + message_header_size + header_header_size,
           properties,
           properties_size
        );

    cs->sendline[
        message_header_size +
        header_header_size +
        properties_size
        ] = 0xce;

    write(
        cs->connfd,
        cs->sendline,
        message_header_size + header_header_size + properties_size + 1
        );

    log_message_header('S', message_header);
}

void send_body(
    connection_state *cs,
    int channel,
    char *payload,
    size_t n
    ) {

    size_t message_header_size = sizeof(amqp_message_header);

    amqp_message_header message_header = {
        .msg_type = BODY,
        .channel = channel,
        .length = n
    };

    unparse_message_header(message_header, cs->sendline);
    memcpy(cs->sendline + message_header_size, payload, n);
    cs->sendline[message_header_size + n] = 0xce;

    write(
        cs->connfd,
        cs->sendline,
        message_header_size + n + 1
        );

    log_message_header('S', message_header);
}
