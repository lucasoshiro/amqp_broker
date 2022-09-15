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

void state_machine_main(int connfd, int thread_id, shared_state *ss) {
    machine_state m = WAIT;

    connection_state cs = {
        .connfd = connfd,
        .thread_id = thread_id,
        .ss = ss
    };

    cs.current_queue_name[0] = '\0';

    strcpy(cs.error_msg, "Unknown");

    while (m != FINISHED && m != FAIL) {
        m = actions[m](&cs);
    
        switch (m) {
        case FAIL:
            log_fail(&cs);
            break;

        case FINISHED:
            log_finished(&cs);
            break;

        default:
            (void) m;
            break;
        }
    }
}

static machine_state action_wait(connection_state *cs) {
    amqp_protocol_header header;

    log_state("WAITING HEADER", cs);

    if (read_protocol_header(cs, &header)) {
        strcpy(cs->error_msg, "Error reading protocol header");
        return FAIL;
    }

    return HEADER_RECEIVED;
}

static machine_state action_header_received(connection_state *cs) {
    log_state("HEADER RECEIVED", cs);

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

    log_state("WAIT START OK", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading protocol header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL)
        return FAIL;

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_START_OK) {
        strcpy(cs->error_msg, "Did not receive connection start ok");
        next_state = FAIL;
    }
    else
        next_state = START_OK_RECEIVED;

    free(method);
    return next_state;
}

static machine_state action_start_ok_received(connection_state *cs) {
    log_state("START OK RECEIVED", cs);

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

    log_state("WAIT TUNE OK", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL){
        strcpy(cs->error_msg, "Error reading method");
        return FAIL;
    }

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_TUNE_OK) {
        strcpy(cs->error_msg, "Did not receive tune ok");
        next_state = FAIL;
    }
    else next_state = WAIT_OPEN_CONNECTION;

    free(method);
    return next_state;
}

static machine_state action_wait_open_connection(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    machine_state next_state;

    log_state("WAIT OPEN CONNECTION", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL) {
        strcpy(cs->error_msg, "Error reading method");
        return FAIL;
    }

    if (method->header.class != CONNECTION ||
        method->header.method != CONNECTION_OPEN) {
        strcpy(cs->error_msg, "Did not receive open");
        next_state = FAIL;
    }

    else
        next_state = OPEN_CONNECTION_RECEIVED;

    free(method);
    return next_state;
}

static machine_state action_open_connection_received(connection_state *cs) {
    log_state("OPEN CONNECTION RECEIVED", cs);

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
    log_state("CLOSE CONNECTION RECEIVED", cs);

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

    log_state("WAIT OPEN CHANNEL", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL) {
        strcpy(cs->error_msg, "Error reading method");
        return FAIL;
    }

    switch (method->header.class) {
    case CHANNEL:
        switch (method->header.method) {
        case CHANNEL_OPEN:
            next_state = OPEN_CHANNEL_RECEIVED;
            break;
        }
        break;

    case CONNECTION:
        switch (method->header.method) {
        case CONNECTION_CLOSE:
            next_state = CLOSE_CONNECTION_RECEIVED;
            break;
        }
        break;
    }

    if (next_state == FAIL) {
        sprintf(
            cs->error_msg,
            "Unexpected class %d method %d",
            method->header.class,
            method->header.method
            );
    }

    free(method);
    return next_state;
}

static machine_state action_open_channel_received(connection_state *cs) {
    log_state("OPEN CHANNEL RECEIVED", cs);

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
    log_state("CLOSE CHANNEL RECEIVED", cs);

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

    log_state("WAIT FUNCTIONAL", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL) {
        strcpy(cs->error_msg, "Error reading method");
        return FAIL;
    }

    switch (method->header.class) {
    case CHANNEL:
        switch (method->header.method) {
        case CHANNEL_CLOSE:
            next_state = CLOSE_CHANNEL_RECEIVED;
            break;
        }

    case CONNECTION:
        switch (method->header.method) {
        case CONNECTION_CLOSE:
            next_state = CLOSE_CONNECTION_RECEIVED;
            break;
        }

    case QUEUE:
        switch (method->header.method) {
        case QUEUE_DECLARE:
            parse_queue_declare_args(method->arguments, cs->current_queue_name);
            log_queue_creation(cs->current_queue_name, cs);
            create_queue(&cs->ss->q_pool, cs->current_queue_name);
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

    if (next_state == FAIL) {
        sprintf(
            cs->error_msg,
            "Unexpected class %d method %d",
            method->header.class,
            method->header.method
            );
        next_state = FAIL;
    }

    free(method);

    return next_state;
}

static machine_state action_queue_declare_received(connection_state *cs) {
    log_state("QUEUE DECLARE RECEIVED", cs);

    int q_size = queue_size(&cs->ss->q_pool, cs->current_queue_name);

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
    log_state("BASIC PUBLISH RECEIVED", cs);
    return WAIT_PUBLISH_CONTENT_HEADER;
}

static machine_state action_wait_publish_content_header(connection_state *cs) {
    amqp_message_header message_header;
    amqp_content_header *content_header;

    log_state("WAIT PUBLISH HEADER", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    content_header = read_content_header(cs, message_header.length);

    free(content_header);

    return WAIT_PUBLISH_CONTENT;
}

static machine_state action_wait_publish_content(connection_state *cs) {
    amqp_message_header message_header;
    amqp_method *method;
    char *body;
    machine_state next_state = FAIL;
    log_state("WAIT PUBLISH CONTENT", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    switch (message_header.msg_type) {
    case METHOD:
        if ((method = read_method(cs, message_header.length)) == NULL) {
            strcpy(cs->error_msg, "Error reading method");
            return FAIL;
        }

        switch (method->header.class) {
        case CHANNEL:
            switch (method->header.method) {
            case CHANNEL_CLOSE:
                next_state = CLOSE_CHANNEL_RECEIVED;
                break;
            }
            break;

        case BASIC:
            switch (method->header.method) {
            case BASIC_PUBLISH:
                parse_basic_publish_args(method->arguments, cs->current_queue_name);
                next_state = BASIC_PUBLISH_RECEIVED;
                break;
            }
            break;

        case CONNECTION:
            switch (method->header.method) {
            case CONNECTION_CLOSE:
                next_state = CLOSE_CONNECTION_RECEIVED;
                break;
            }
        }

        if (next_state == FAIL) {
            sprintf(
                cs->error_msg,
                "Unexpected class %d method %d",
                method->header.class,
                method->header.method
                );
            next_state = FAIL;
        }

        free(method);
        break;

    case BODY:
        body = read_body(cs, message_header.length);

        enqueue_to(&cs->ss->q_pool, cs->current_queue_name, body);
        log_enqueue(cs, body);

        next_state = WAIT_PUBLISH_CONTENT;
        free(body);
        break;

    default:
        sprintf(
            cs->error_msg,
            "Unexpected message type %d",
            message_header.msg_type
            );
        next_state = FAIL;
    }

    return next_state;
}

static machine_state action_basic_consume_received(connection_state *cs) {
    log_state("BASIC CONSUME RECEIVED", cs);

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

    log_state("WAIT VALUE DEQUEUE", cs);

    while (queue_size(&cs->ss->q_pool, cs->current_queue_name) == 0)
        sleep(1);

    s = dequeue_from(&cs->ss->q_pool, cs->current_queue_name);
    strcpy(cs->recvline, s);
    free(s);

    log_dequeue(cs, cs->recvline);

    return VALUE_DEQUEUE_RECEIVED;
}

static machine_state action_value_dequeue_received(connection_state *cs) {
    size_t body_size = strlen(cs->recvline);

    log_state("VALUE DEQUEUE RECEIVED", cs);

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

    log_state("WAIT CONSUME ACK", cs);

    if (read_message_header(cs, &message_header)) {
        strcpy(cs->error_msg, "Error reading message header");
        return FAIL;
    }

    if ((method = read_method(cs, message_header.length)) == NULL) {
        strcpy(cs->error_msg, "Error reading method");
        return FAIL;
    }

    if (method->header.class != BASIC ||
        method->header.method != BASIC_ACK) {
            sprintf(
                cs->error_msg,
                "Unexpected class %d method %d",
                method->header.class,
                method->header.method
                );
        next_state = FAIL;
    }
    else
        next_state = WAIT_VALUE_DEQUEUE;

    free(method);
    return next_state;
}

static machine_state action_noop(connection_state *cs) {
    (void) cs;
    return FAIL;
}
