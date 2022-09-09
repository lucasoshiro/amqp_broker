#include "state_machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "amqp_message.h"
#include "connection_state.h"
#include "hardcoded_values.h"
#include "log.h"
#include "queue_pool.h"

// Actions

// Connection
static machine_state action_wait(connection_state *);
static machine_state action_header_received(connection_state *);
static machine_state action_wait_start_ok(connection_state *);
static machine_state action_start_ok_received(connection_state *);
static machine_state action_wait_tune_ok(connection_state *);
static machine_state action_wait_open_connection(connection_state *);
static machine_state action_open_connection_received(connection_state *);
static machine_state action_close_connection_received(connection_state *);

// Channel
static machine_state action_wait_open_channel(connection_state *);
static machine_state action_open_channel_received(connection_state *);
static machine_state action_close_channel_received(connection_state *);

// Functional
static machine_state action_wait_functional(connection_state *);
static machine_state action_queue_declare_received(connection_state *);

// Publish
static machine_state action_basic_publish_received(connection_state *);
static machine_state action_wait_publish_content_header(connection_state *);
static machine_state action_wait_publish_content(connection_state *);

// Consume
static machine_state action_basic_consume_received(connection_state *);
static machine_state action_wait_value_dequeue(connection_state *);
static machine_state action_value_dequeue_received(connection_state *);
static machine_state action_wait_consume_ack(connection_state *);

// No operation
static machine_state action_noop(connection_state *);

machine_state (*actions[NUM_STATES])(connection_state *) = {
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
    action_wait_consume_ack,

    // Finish
    action_noop,
    action_noop
};

void state_machine_main(int connfd, pthread_t *thread, shared_state *ss) {
    machine_state m = WAIT;

    connection_state cs = {
        .connfd = connfd,
        .thread = thread,
        .ss = ss
    };

    cs.current_queue_name[0] = '\0';

    while (m != FINISHED && m != FAIL) {
        m = actions[m](&cs);
        if (m == FAIL)
            printf("!!!!!!!!!!!! FAIL !!!!!!!!!!!!!!\n");
    }
}

static machine_state action_wait(connection_state *cs) {
    amqp_protocol_header header;

    log_state("WAITING HEADER");

    if (read_protocol_header(cs, &header))
        return FAIL;

    return HEADER_RECEIVED;
}

static machine_state action_header_received(connection_state *cs) {
    log_state("HEADER RECEIVED");

    send_method(
        cs,
        CONNECTION,
        CONNECTION_START,
        0,
        (void *) CONNECTION_START_ARGS,
        CONNECTION_START_ARGS_SIZE
        );

    return WAIT_START_OK;
}

static machine_state action_wait_start_ok(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state;

    log_state("WAIT START OK");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_START_OK)
        next_state = FAIL;
    else
        next_state = START_OK_RECEIVED;

    free(method);
    return next_state;
}

static machine_state action_start_ok_received(connection_state *cs) {
    log_state("START OK RECEIVED");

    send_method(
        cs,
        CONNECTION,
        CONNECTION_TUNE,
        0,
        (void *) CONNECTION_TUNE_ARGS,
        CONNECTION_TUNE_ARGS_SIZE
        );
    return WAIT_TUNE_OK;
}

static machine_state action_wait_tune_ok(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state;

    log_state("WAIT TUNE OK");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_TUNE_OK)
        next_state = FAIL;
    else next_state = WAIT_OPEN_CONNECTION;

    free(method);
    return next_state;
}

static machine_state action_wait_open_connection(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state;

    log_state("WAIT OPEN CONNECTION");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_OPEN)
        next_state = FAIL;
    else
        next_state = OPEN_CONNECTION_RECEIVED;

    free(method);
    return next_state;
}

static machine_state action_open_connection_received(connection_state *cs) {
    log_state("OPEN CONNECTION RECEIVED");

    send_method(
        cs,
        CONNECTION,
        CONNECTION_OPEN_OK,
        0,
        (void *) CONNECTION_OPEN_OK_ARGS,
        CONNECTION_OPEN_OK_ARGS_SIZE
        );

    return WAIT_OPEN_CHANNEL;
}

static machine_state action_close_connection_received(connection_state *cs) {
    log_state("CLOSE CONNECTION RECEIVED");

    send_method(
        cs,
        CONNECTION,
        CONNECTION_CLOSE_OK,
        0,
        (void *) ARGS_EMPTY,
        ARGS_EMPTY_SIZE
        );

    return FINISHED;
}

static machine_state action_wait_open_channel(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;

    log_state("WAIT OPEN CHANNEL");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

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

static machine_state action_open_channel_received(connection_state *cs) {
    log_state("OPEN CHANNEL RECEIVED");

    send_method(
        cs,
        CHANNEL,
        CHANNEL_OPEN_OK,
        1,
        (void *) CHANNEL_OPEN_OK_ARGS,
        CHANNEL_OPEN_OK_ARGS_SIZE
        );

    return WAIT_FUNCTIONAL;
}

static machine_state action_close_channel_received(connection_state *cs) {
    log_state("CLOSE CHANNEL RECEIVED");

    send_method(
        cs,
        CHANNEL,
        CHANNEL_CLOSE_OK,
        1,
        (void *) ARGS_EMPTY,
        ARGS_EMPTY_SIZE
        );

    return WAIT_OPEN_CHANNEL;
}

static machine_state action_wait_functional(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;

    log_state("WAIT FUNCTIONAL");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

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
            parse_queue_declare_args(method->arguments, cs->current_queue_name);
            create_queue(&cs->ss->pool, cs->current_queue_name);
            next_state = QUEUE_DECLARE_RECEIVED;
            break;
        }
        break;

    case BASIC:
        switch (method->header.method) {
        case BASIC_PUBLISH:
            parse_basic_publish_args(method->arguments, cs->current_queue_name);
            next_state = BASIC_PUBLISH_RECEIVED;
            break;

        case BASIC_CONSUME:
            parse_basic_consume_args(method->arguments, cs->current_queue_name);
            next_state = BASIC_CONSUME_RECEIVED;
            break;
        }
    }

    free(method);

    return next_state;
}

static machine_state action_queue_declare_received(connection_state *cs) {
    log_state("QUEUE DECLARE RECEIVED");

    int q_size = queue_size(&cs->ss->pool, cs->current_queue_name);

    send_queue_declare_ok(
        cs,
        1,                      /* TODO: CHANGE IT!!! */
        cs->current_queue_name,
        q_size,
        0                       /* TODO: CHANGE IT!!! */
        );

    return WAIT_FUNCTIONAL;
}

static machine_state action_basic_publish_received(connection_state *cs) {
    (void) cs;
    log_state("BASIC PUBLISH RECEIVED");
    return WAIT_PUBLISH_CONTENT_HEADER;
}

static machine_state action_wait_publish_content_header(connection_state *cs) {
    amqp_message_header message_header;
    amqp_content_header *content_header;

    log_state("WAIT PUBLISH HEADER");

    if (read_message_header(cs, &message_header)) return FAIL;

    content_header = read_content_header(cs, message_header.length);

    free(content_header);

    return WAIT_PUBLISH_CONTENT;
}

static machine_state action_wait_publish_content(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    char *body;
    machine_state next_state = FAIL;
    log_state("WAIT PUBLISH CONTENT");

    if (read_message_header(cs, &message_header)) return FAIL;

    switch (message_header.msg_type) {
    case METHOD:
        if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;
        next_state =
            (method->header.class == CHANNEL &&
             method->header.method == CHANNEL_CLOSE)
            ? CLOSE_CHANNEL_RECEIVED
            : FAIL;
        free(method);
        break;
    case BODY:
        body = read_body(cs, message_header.length);

        enqueue_to(&cs->ss->pool, cs->current_queue_name, body);

        next_state = WAIT_PUBLISH_CONTENT;
        free(body);
        break;
    }

    return next_state;
}

static machine_state action_basic_consume_received(connection_state *cs) {
    log_state("BASIC CONSUME RECEIVED");

    send_method(
        cs,
        BASIC,
        BASIC_CONSUME_OK,
        1,
        (void *) BASIC_CONSUME_OK_ARGS,
        BASIC_CONSUME_OK_ARGS_SIZE
        );

    return WAIT_VALUE_DEQUEUE;
}

static machine_state action_wait_value_dequeue(connection_state *cs) {
    char *s;

    log_state("WAIT VALUE DEQUEUE");

    while (queue_size(&cs->ss->pool, cs->current_queue_name) == 0)
        sleep(1);

    s = dequeue_from(&cs->ss->pool, cs->current_queue_name);
    strcpy(cs->recvline, s);
    free(s);

    return VALUE_DEQUEUE_RECEIVED;
}

static machine_state action_value_dequeue_received(connection_state *cs) {
    size_t body_size = strlen(cs->recvline);

    log_state("VALUE DEQUEUE RECEIVED");

    send_method(
        cs,
        BASIC,
        BASIC_DELIVER,
        1,
        (void *) BASIC_DELIVER_ARGS,
        BASIC_DELIVER_ARGS_SIZE
        );

    send_content_header(
        cs,
        BASIC,
        1,
        0,
        body_size,
        0x1000,
        (void *) CONTENT_HEADER_PROPERTIES
        );

    send_body(
        cs,
        1,
        cs->recvline,
        strlen(cs->recvline)
        );

    return WAIT_CONSUME_ACK;
}

static machine_state action_wait_consume_ack(connection_state * cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state = FAIL;

    log_state("WAIT CONSUME ACK");

    if (read_message_header(cs, &message_header)) return FAIL;

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

    if (method->header.class != BASIC ||
        method->header.method != BASIC_ACK)
        next_state = FAIL;
    else
        next_state = WAIT_VALUE_DEQUEUE;

    free(method);
    return next_state;
}

static machine_state action_noop(connection_state *cs) {
    (void) cs;
    return FAIL;
}
