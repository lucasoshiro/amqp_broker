#include "state_machine.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "amqp_message.h"
#include "amqp_methods.h"

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
static machine_state action_basic_publish_received();

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
    action_basic_publish_received,

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
    ssize_t n;
    amqp_protocol_header header;

    printf("WAITING HEADER\n");

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

    printf("HEADER RECEIVED\n");
    write(connfd, sendline, n);
    return WAIT_START_OK;
}

static machine_state action_wait_start_ok() {
    ssize_t n;
    amqp_message_header header;

    printf("WAIT START OK\n");

    n = read(connfd, recvline, sizeof(header));

    if (parse_message_header(recvline, n, &header))
        return FAIL;

    print_message_header(header);

    if (header.class != CONNECTION ||
        header.method != CONNECTION_START_OK)
        return FAIL;

    n = read(connfd, recvline, header.length - 4);
    n = read(connfd, recvline, 1);

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

    printf("START OK RECEIVED\n");
    write(connfd, sendline, n);
    return WAIT_TUNE_OK;
}

static machine_state action_wait_tune_ok() {
    ssize_t n;
    amqp_message_header header;

    printf("WAIT TUNE OK\n");

    n = read(connfd, recvline, sizeof(header));

    if (parse_message_header(recvline, n, &header))
        return FAIL;

    print_message_header(header);

    if (header.class != CONNECTION ||
        header.method != CONNECTION_TUNE_OK)
        return FAIL;

    n = read(connfd, recvline, header.length - 4);
    n = read(connfd, recvline, 1);

    return WAIT_OPEN_CONNECTION;
}

static machine_state action_wait_open_connection() {
    ssize_t n;
    amqp_message_header header;

    printf("WAIT OPEN CONNECTION\n");

    n = read(connfd, recvline, sizeof(header));

    if (parse_message_header(recvline, n, &header))
        return FAIL;

    print_message_header(header);

    if (header.class != CONNECTION ||
        header.method != CONNECTION_OPEN)
        return FAIL;

    n = read(connfd, recvline, header.length - 4);
    n = read(connfd, recvline, 1);

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

    printf("OPEN CONNECTION RECEIVED\n");
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

    printf("CLOSE CONNECTION RECEIVED\n");
    write(connfd, sendline, n);
    return FINISHED;
}

static machine_state action_wait_open_channel() {
    ssize_t n;
    amqp_message_header header;
    machine_state next_state = FAIL;

    printf("WAIT OPEN CHANNEL\n");

    n = read(connfd, recvline, sizeof(header));

    if (parse_message_header(recvline, n, &header))
        return FAIL;

    print_message_header(header);

    switch (header.class) {
    case CHANNEL:
        switch (header.method) {
        case CHANNEL_OPEN:
            next_state = OPEN_CHANNEL_RECEIVED;
            break;
        }

    case CONNECTION:
        switch (header.method) {
        case CONNECTION_CLOSE:
            next_state = CLOSE_CONNECTION_RECEIVED;
            break;
        }
    }

    n = read(connfd, recvline, header.length - 4);
    n = read(connfd, recvline, 1);

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

    printf("OPEN CHANNEL RECEIVED\n");
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

    printf("CLOSE CHANNEL RECEIVED\n");
    write(connfd, sendline, n);
    return WAIT_OPEN_CHANNEL;
}

static machine_state action_wait_functional() {
    ssize_t n;
    amqp_message_header header;
    machine_state next_state = FAIL;

    printf("WAIT FUNCTIONAL\n");

    n = read(connfd, recvline, sizeof(header));

    if (parse_message_header(recvline, n, &header))
        return FAIL;

    print_message_header(header);

    n = read(connfd, recvline, header.length - 4);
    n = read(connfd, recvline, 1);

    switch (header.class) {
    case CHANNEL:
        switch (header.method) {
        case CHANNEL_CLOSE:
            next_state = CLOSE_CHANNEL_RECEIVED;
            break;
        }

    case QUEUE:
        switch (header.method) {
        case QUEUE_DECLARE:
            next_state = QUEUE_DECLARE_RECEIVED;
            break;
        }
        break;

    case BASIC:
        switch (header.method) {
        case BASIC_PUBLISH:
            next_state = BASIC_PUBLISH_RECEIVED;
            break;
        }
    }

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

    printf("QUEUE DECLARE RECEIVED\n");
    write(connfd, sendline, n);
    return WAIT_FUNCTIONAL;
}

static machine_state action_basic_publish_received() {
    printf("BASIC PUBLISH RECEIVED\n");
    return WAIT_FUNCTIONAL;
}

static machine_state action_noop() {
    return FAIL;
}
