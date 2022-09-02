#include "state_machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "amqp_message.h"
#include "amqp_methods.h"
#include "log.h"

#define MAXLINE 4096

int connfd;
char recvline[MAXLINE + 1];
char sendline[MAXLINE + 1];

// Actions

// Connection
static machine_state action_wait();
static machine_state action_header_received();
static machine_state action_wait_start_ok();
static machine_state action_start_ok_received();
static machine_state action_wait_tune_ok();
static machine_state action_wait_open_connection();
static machine_state action_open_connection_received();
static machine_state action_close_connection_received();

// Channel
static machine_state action_wait_open_channel();
static machine_state action_open_channel_received();
static machine_state action_close_channel_received();

// Functional
static machine_state action_wait_functional();
static machine_state action_queue_declare_received();

// Publish
static machine_state action_basic_publish_received();
static machine_state action_wait_publish_content_header();
static machine_state action_wait_publish_content();

// Consume
static machine_state action_basic_consume_received();
static machine_state action_wait_value_dequeue();
static machine_state action_value_dequeue_received();

// No operation
static machine_state action_noop();

machine_state (*actions[NUM_STATES])() = {
    // Connection
    action_wait,
    action_header_received,
    action_wait_start_ok,
    action_start_ok_received,
    action_wait_tune_ok,
    action_wait_open_connection,
    action_open_connection_received,
    action_close_connection_received,

    // Channel
    action_wait_open_channel,
    action_open_channel_received,
    action_close_channel_received,

    // Functional
    action_wait_functional,
    action_queue_declare_received,

    // Publish
    action_basic_publish_received,
    action_wait_publish_content_header,
    action_wait_publish_content,

    // Consume
    action_basic_consume_received,
    action_wait_value_dequeue,
    action_value_dequeue_received,

    // Finish
    action_noop,
    action_noop
};

void state_machine_main(int _connfd) {
    machine_state m = WAIT;

    connfd = _connfd;

    while (m != FINISHED && m != FAIL) {
        m = actions[m]();
        if (m == FAIL)
            printf("!!!!!!!!!!!! FAIL !!!!!!!!!!!!!!\n");
    }
}

static machine_state action_wait() {
    size_t n;
    amqp_protocol_header header;

    log_state("WAITING HEADER");

    n = read(connfd, recvline, sizeof(header));

    if (parse_protocol_header(recvline, n, &header))
        return FAIL;

    return HEADER_RECEIVED;
}

static machine_state action_header_received() {
    char dummy_argument_str[] =
        "\x00\x09\x00\x00\x01\xd2\x0c\x63\x61\x70\x61\x62\x69\x6c\x69\x74"
        "\x69\x65\x73\x46\x00\x00\x00\xc7\x12\x70\x75\x62\x6c\x69\x73\x68"
        "\x65\x72\x5f\x63\x6f\x6e\x66\x69\x72\x6d\x73\x74\x01\x1a\x65\x78"
        "\x63\x68\x61\x6e\x67\x65\x5f\x65\x78\x63\x68\x61\x6e\x67\x65\x5f"
        "\x62\x69\x6e\x64\x69\x6e\x67\x73\x74\x01\x0a\x62\x61\x73\x69\x63"
        "\x2e\x6e\x61\x63\x6b\x74\x01\x16\x63\x6f\x6e\x73\x75\x6d\x65\x72"
        "\x5f\x63\x61\x6e\x63\x65\x6c\x5f\x6e\x6f\x74\x69\x66\x79\x74\x01"
        "\x12\x63\x6f\x6e\x6e\x65\x63\x74\x69\x6f\x6e\x2e\x62\x6c\x6f\x63"
        "\x6b\x65\x64\x74\x01\x13\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x70"
        "\x72\x69\x6f\x72\x69\x74\x69\x65\x73\x74\x01\x1c\x61\x75\x74\x68"
        "\x65\x6e\x74\x69\x63\x61\x74\x69\x6f\x6e\x5f\x66\x61\x69\x6c\x75"
        "\x72\x65\x5f\x63\x6c\x6f\x73\x65\x74\x01\x10\x70\x65\x72\x5f\x63"
        "\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x71\x6f\x73\x74\x01\x0f\x64\x69"
        "\x72\x65\x63\x74\x5f\x72\x65\x70\x6c\x79\x5f\x74\x6f\x74\x01\x0c"
        "\x63\x6c\x75\x73\x74\x65\x72\x5f\x6e\x61\x6d\x65\x53\x00\x00\x00"
        "\x10\x72\x61\x62\x62\x69\x74\x40\x6d\x79\x2d\x72\x61\x62\x62\x69"
        "\x74\x09\x63\x6f\x70\x79\x72\x69\x67\x68\x74\x53\x00\x00\x00\x37"
        "\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20\x28\x63\x29\x20\x32\x30"
        "\x30\x37\x2d\x32\x30\x32\x32\x20\x56\x4d\x77\x61\x72\x65\x2c\x20"
        "\x49\x6e\x63\x2e\x20\x6f\x72\x20\x69\x74\x73\x20\x61\x66\x66\x69"
        "\x6c\x69\x61\x74\x65\x73\x2e\x0b\x69\x6e\x66\x6f\x72\x6d\x61\x74"
        "\x69\x6f\x6e\x53\x00\x00\x00\x39\x4c\x69\x63\x65\x6e\x73\x65\x64"
        "\x20\x75\x6e\x64\x65\x72\x20\x74\x68\x65\x20\x4d\x50\x4c\x20\x32"
        "\x2e\x30\x2e\x20\x57\x65\x62\x73\x69\x74\x65\x3a\x20\x68\x74\x74"
        "\x70\x73\x3a\x2f\x2f\x72\x61\x62\x62\x69\x74\x6d\x71\x2e\x63\x6f"
        "\x6d\x08\x70\x6c\x61\x74\x66\x6f\x72\x6d\x53\x00\x00\x00\x11\x45"
        "\x72\x6c\x61\x6e\x67\x2f\x4f\x54\x50\x20\x32\x35\x2e\x30\x2e\x34"
        "\x07\x70\x72\x6f\x64\x75\x63\x74\x53\x00\x00\x00\x08\x52\x61\x62"
        "\x62\x69\x74\x4d\x51\x07\x76\x65\x72\x73\x69\x6f\x6e\x53\x00\x00"
        "\x00\x06\x33\x2e\x31\x30\x2e\x37\x00\x00\x00\x0e\x41\x4d\x51\x50"
        "\x4c\x41\x49\x4e\x20\x50\x4c\x41\x49\x4e\x00\x00\x00\x05\x65\x6e"
        "\x5f\x55\x53";

    int n = prepare_message(
        CONNECTION,
        CONNECTION_START,
        0,
        dummy_argument_str,
        499,
        sendline
        );

    log_state("HEADER RECEIVED");
    write(connfd, sendline, n);
    return WAIT_START_OK;
}

static machine_state action_wait_start_ok() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;

    log_state("WAIT START OK");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    method = parse_method(recvline, n);
    n = read(connfd, recvline, 1);

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_START_OK)
        return FAIL;

    free(method);
    return START_OK_RECEIVED;
}

static machine_state action_start_ok_received() {
    char dummy_argument_str[] = "\x07\xff\x00\x02\x00\x00\x00\x3c";

    int n = prepare_message(
        CONNECTION,
        CONNECTION_TUNE,
        0,
        dummy_argument_str,
        8,
        sendline
        );

    log_state("START OK RECEIVED");
    write(connfd, sendline, n);
    return WAIT_TUNE_OK;
}

static machine_state action_wait_tune_ok() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;

    log_state("WAIT TUNE OK");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    method = parse_method(recvline, n);
    n = read(connfd, recvline, 1);

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_TUNE_OK)
        return FAIL;

    free(method);
    return WAIT_OPEN_CONNECTION;
}

static machine_state action_wait_open_connection() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;

    log_state("WAIT OPEN CONNECTION");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    method = parse_method(recvline, n);
    n = read(connfd, recvline, 1);

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_OPEN)
        return FAIL;

    free(method);
    return OPEN_CONNECTION_RECEIVED;
}

static machine_state action_open_connection_received() {
    char dummy_argument_str[] = "\x00";

    int n = prepare_message(
        CONNECTION,
        CONNECTION_OPEN_OK,
        0,
        dummy_argument_str,
        1,
        sendline
        );

    log_state("OPEN CONNECTION RECEIVED");
    write(connfd, sendline, n);
    return WAIT_OPEN_CHANNEL;
}

static machine_state action_close_connection_received() {
    char dummy_argument_str[] = "";

    int n = prepare_message(
        CONNECTION,
        CONNECTION_CLOSE_OK,
        0,
        dummy_argument_str,
        0,
        sendline
        );

    log_state("CLOSE CONNECTION RECEIVED");
    write(connfd, sendline, n);
    return FINISHED;
}

static machine_state action_wait_open_channel() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;

    log_state("WAIT OPEN CHANNEL");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    method = parse_method(recvline, n);
    n = read(connfd, recvline, 1);

    switch (method->header.class) {
    case CHANNEL:
        switch (method->header.method) {
        case CHANNEL_OPEN:
            next_state = OPEN_CHANNEL_RECEIVED;
            break;
        }

    case CONNECTION:
        switch (method->header.method) {
        case CONNECTION_CLOSE:
            next_state = CLOSE_CONNECTION_RECEIVED;
            break;
        }
    }

    free(method);
    return next_state;
}

static machine_state action_open_channel_received() {
    char dummy_argument_str[] = "\x00\x00\x00\x00";

    int n = prepare_message(
        CHANNEL,
        CHANNEL_OPEN_OK,
        1,
        dummy_argument_str,
        4,
        sendline
        );

    log_state("OPEN CHANNEL RECEIVED");
    write(connfd, sendline, n);
    return WAIT_FUNCTIONAL;
}

static machine_state action_close_channel_received() {
    char dummy_argument_str[] = "";

    int n = prepare_message(
        CHANNEL,
        CHANNEL_CLOSE_OK,
        1,
        dummy_argument_str,
        0,
        sendline
        );

    log_state("CLOSE CHANNEL RECEIVED");
    write(connfd, sendline, n);
    return WAIT_OPEN_CHANNEL;
}

static machine_state action_wait_functional() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;

    log_state("WAIT FUNCTIONAL");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    method = parse_method(recvline, n);
    n = read(connfd, recvline, 1);

    switch (method->header.class) {
    case CHANNEL:
        switch (method->header.method) {
        case CHANNEL_CLOSE:
            next_state = CLOSE_CHANNEL_RECEIVED;
            break;
        }

    case QUEUE:
        switch (method->header.method) {
        case QUEUE_DECLARE:
            next_state = QUEUE_DECLARE_RECEIVED;
            break;
        }
        break;

    case BASIC:
        switch (method->header.method) {
        case BASIC_PUBLISH:
            next_state = BASIC_PUBLISH_RECEIVED;
            break;
        case BASIC_CONSUME:
            next_state = BASIC_CONSUME_RECEIVED;
            break;
        }
    }

    free(method);

    return next_state;
}

static machine_state action_queue_declare_received() {
    char dummy_argument_str[] = "\x07\x63\x68\x65\x65\x74\x6f\x73\x00\x00\x00\x00\x00\x00\x00\x00";

    int n = prepare_message(
        QUEUE,
        QUEUE_DECLARE_OK,
        1,
        dummy_argument_str,
        16,
        sendline
        );

    log_state("QUEUE DECLARE RECEIVED");
    write(connfd, sendline, n);
    return WAIT_FUNCTIONAL;
}

static machine_state action_basic_publish_received() {
    log_state("BASIC PUBLISH RECEIVED");
    return WAIT_PUBLISH_CONTENT_HEADER;
}

static machine_state action_wait_publish_content_header() {
    size_t n;
    amqp_message_header message_header;
    amqp_content_header *content_header;

    log_state("WAIT PUBLISH HEADER");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);
    content_header = parse_content_header(recvline, n);
    n = read(connfd, recvline, 1);

    free(content_header);

    return WAIT_PUBLISH_CONTENT;
}

static machine_state action_wait_publish_content() {
    size_t n;
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;
    log_state("WAIT PUBLISH CONTENT");

    n = read(connfd, recvline, sizeof(message_header));
    if (parse_message_header(recvline, n, &message_header)) return FAIL;
    log_message_header('C', message_header);

    n = read(connfd, recvline, message_header.length);

    switch (message_header.msg_type) {
    case METHOD:
        method = parse_method(recvline, n);
        next_state =
            (method->header.class == CHANNEL &&
             method->header.method == CHANNEL_CLOSE)
            ? CLOSE_CHANNEL_RECEIVED
            : FAIL;
        free(method);
        break;
    case BODY:
        next_state = WAIT_PUBLISH_CONTENT;
        break;
    }

    n = read(connfd, recvline, 1);
    return next_state;
}

static machine_state action_basic_consume_received() {
    char dummy_argument_str[] = "\x00\x00\x00\x00\x00\x00\x00\x01\x00";

    int n = prepare_message(
        BASIC,
        BASIC_CONSUME_OK,
        1,
        dummy_argument_str,
        13,
        sendline
        );

    log_state("BASIC CONSUME RECEIVED");
    write(connfd, sendline, n);
    return WAIT_VALUE_DEQUEUE;
}

static machine_state action_wait_value_dequeue() {
    // TODO: dequeue
    return VALUE_DEQUEUE_RECEIVED;
}

static machine_state action_value_dequeue_received() {
    int n;
    char dummy_deliver_argument_str[] =
        "\x1f\x61\x6d\x71\x2e\x63\x74\x61\x67\x2d\x56\x64\x34\x59\x53\x35"
        "\x52\x49\x32\x34\x5f\x2d\x71\x48\x68\x61\x6e\x51\x4e\x51\x4a\x67"
        "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x07\x63\x68\x65\x65\x74"
        "\x6f\x73";

    char dummy_content_header_properties[] = "\x01";

    char dummy_payload[7] = "\x62\x61\x74\x61\x74\x61";

    log_state("VALUE DEQUEUE RECEIVED");

    n = prepare_message(
        BASIC,
        BASIC_DELIVER,
        1,
        dummy_deliver_argument_str,
        50,
        sendline
        );

    write(connfd, sendline, n);

    n = prepare_content_header(
        BASIC,
        1,
        0,
        6,
        0x1000,
        dummy_content_header_properties,
        sendline
        );

    write(connfd, sendline, n);

    n = prepare_content_body(
        1,
        dummy_payload,
        6,
        sendline
        );

    write(connfd, sendline, n);

    return WAIT_VALUE_DEQUEUE;
}

static machine_state action_noop() {
    return FAIL;
}
