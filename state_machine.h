#pragma once

typedef enum machine_state {
    // Connection
    WAIT,
    HEADER_RECEIVED,
    WAIT_START_OK,
    START_OK_RECEIVED,
    WAIT_TUNE_OK,
    WAIT_OPEN_CONNECTION,
    OPEN_CONNECTION_RECEIVED,
    CLOSE_CONNECTION_RECEIVED,

    // Channel
    WAIT_OPEN_CHANNEL,
    OPEN_CHANNEL_RECEIVED,
    CLOSE_CHANNEL_RECEIVED,

    // Functional
    WAIT_FUNCTIONAL,
    QUEUE_DECLARE_RECEIVED,

    // Publish
    BASIC_PUBLISH_RECEIVED,
    WAIT_PUBLISH_CONTENT_HEADER,
    WAIT_PUBLISH_CONTENT,

    // Consume
    BASIC_CONSUME_RECEIVED,
    WAIT_VALUE_DEQUEUE,
    VALUE_DEQUEUE_RECEIVED,
    WAIT_ACK,

    // Finish states
    FINISHED,
    FAIL,

    // Count
    NUM_STATES
} machine_state;

void state_machine_main(int _connfd);
