#include "amqp_message.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "log.h"

const amqp_protocol_header default_amqp_header = {
    .amqp = {'A', 'M', 'Q', 'P'},
    .id_major = 0,
    .id_minor = 0,
    .version_major = 9,
    .version_minor = 1
};

static size_t fill_short_string(char *, amqp_short_string *);
static int parse_protocol_header(char *, size_t, amqp_protocol_header *);
static int parse_message_header(char *, size_t, amqp_message_header *);
static void unparse_message_header(amqp_message_header, char *);
static void unparse_method_header(amqp_method_header, char *);
static void unparse_content_header_header(amqp_content_header_header, char *);

static size_t fill_short_string(char *str, amqp_short_string *amqp_str) {
    size_t size = strlen(str);
    amqp_str->size = size;
    strcpy(amqp_str->str, str);
    return size;
}

void parse_queue_declare_args(void *args, char *queue_name) {
    char *args_str = args;
    amqp_short_string *short_str = (void *) (args_str + 2); /* Drop ticket */
    memcpy(queue_name, short_str->str, short_str->size);

    queue_name[short_str->size] = '\0';
}

void parse_basic_publish_args(void *args, char *queue_name) {
    char *args_str = args;
    amqp_short_string *exchange_short_str =
        (void *) (args_str + 2);                            /* Drop ticket */

    amqp_short_string *queue_short_str =
        (void *) (args_str + 3 + exchange_short_str->size); /* Drop tick and exchange */

    strncpy(queue_name, queue_short_str->str, queue_short_str->size);

    queue_name[queue_short_str->size] = '\0';
}

void parse_basic_consume_args(void *args, char *queue_name) {
    char *args_str = args;
    amqp_short_string *short_str = (void *) (args_str + 2); /* Drop ticket and exchange*/
    strncpy(queue_name, short_str->str, short_str->size);

    queue_name[short_str->size] = '\0';
}

static int parse_protocol_header(char *s, size_t n, amqp_protocol_header *header) {
    size_t header_size = sizeof(amqp_protocol_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);

    return memcmp(header, &default_amqp_header, header_size);
}

int read_protocol_header(connection_state *cs, amqp_protocol_header *header) {
    size_t n = read_until(cs->connfd, cs->recvline, sizeof(*header));
    return parse_protocol_header(cs->recvline, n, header);
}

static int parse_message_header(char *s, size_t n, amqp_message_header *header) {
    size_t header_size = sizeof(amqp_message_header);

    if (n < header_size) return 1;
    memcpy(header, s, header_size);

    header->channel = ntohs(header->channel);
    header->length = ntohl(header->length);
    return 0;
}

static void unparse_message_header(amqp_message_header header, char *s) {
    header.channel = htons(header.channel);
    header.length = htonl(header.length);

    memcpy(s, &header, sizeof(amqp_message_header));
}

int read_message_header(connection_state *cs, amqp_message_header *header) {
    size_t n;
    n = read_until(cs->connfd, cs->recvline, sizeof(*header));
    if (parse_message_header(cs->recvline, n, header)) return 1;
    log_message_header('C', *header, cs);
    return 0;
}

static void unparse_method_header(amqp_method_header header, char *s) {
    header.class = htons(header.class);
    header.method = htons(header.method);

    memcpy(s, &header, sizeof(amqp_method_header));
}

static amqp_method *parse_method(char *s, size_t n, amqp_method *method) {
    size_t header_size = sizeof(amqp_method_header);

    if (n < header_size) return NULL;

    memcpy(method, s, header_size + n - 1);

    method->header.class = ntohs(method->header.class);
    method->header.method = ntohs(method->header.method);

    return method;
}

amqp_method *read_method(connection_state *cs, int length) {
    size_t n;
    amqp_method *method;

    if ((n = read_until(cs->connfd, cs->recvline, length)) == 0)
        return NULL;

    if ((method = parse_method(cs->recvline, n, (void *) cs->parsed)) == NULL)
        return NULL;

    if (read_until(cs->connfd, cs->recvline, 1) == 0)
        return NULL;

    return method;
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

    cs->sendline[header_size + args_size] = '\xce';
    write(cs->connfd, cs->sendline, header_size + args_size + 1);

    log_message_header('S', message_header, cs);
}

void send_queue_declare_ok(
    connection_state *cs,
    uint16_t channel,
    char *queue_name,
    uint32_t message_count,
    uint32_t consumer_count
    ) {

    char arguments[300];
    size_t name_size;
    size_t args_size;
    amqp_short_string *_queue_name;

    struct {
        uint32_t message;
        uint32_t consumer;
    } count;

    _queue_name = (void *) arguments;
    name_size = fill_short_string(queue_name, _queue_name);

    count.message = htonl(message_count);
    count.consumer = htonl(consumer_count);

    memcpy(
        arguments + sizeof(amqp_short_string) + name_size - 1,
        &count,
        sizeof(count)
        );
    args_size = sizeof(amqp_short_string) + name_size - 1 + sizeof(count);

    send_method(cs, QUEUE, QUEUE_DECLARE_OK, channel, arguments, args_size);
}

static void unparse_content_header_header(
    amqp_content_header_header header,
    char *s
    ) {
    header.class = htons(header.class);
    header.weight = htons(header.weight);
    header.body_size = swipe_endianness_64(header.body_size);
    header.property_flags = htons(header.property_flags);

    memcpy(s, &header, sizeof(header));
}

static amqp_content_header *parse_content_header(
    char *s,
    size_t n,
    amqp_content_header *content_header
    ) {
    size_t header_size = sizeof(amqp_content_header_header);

    if (n < header_size) return NULL;

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
    n = read_until(cs->connfd, cs->recvline, length);
    header = parse_content_header(cs->recvline, length, (void *) cs->parsed);
    n = read_until(cs->connfd, cs->recvline, 1);

    return n == 0 ? NULL : header;
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
        ] = '\xce';

    write(
        cs->connfd,
        cs->sendline,
        message_header_size + header_header_size + properties_size + 1
        );

    log_message_header('S', message_header, cs);
}

char *read_body(connection_state *cs, int length) {
    int n = read_until(cs->connfd, cs->recvline, length + 1);
    char *body = cs->parsed;
    memcpy(body, cs->recvline, n);
    body[length] = '\0';
    return body;
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
    cs->sendline[message_header_size + n] = '\xce';

    write(
        cs->connfd,
        cs->sendline,
        message_header_size + n + 1
        );

    log_message_header('S', message_header, cs);
}
