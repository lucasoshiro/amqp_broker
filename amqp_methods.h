#pragma once

enum msg_format {
    METHOD = 1,
    CONTENT_HEADER,
    BODY,
    HEARTBEAT
};

enum class {
    CONNECTION = 10,
    CHANNEL    = 20,
    EXCHANGE   = 40,
    QUEUE      = 50,
    BASIC      = 60,
    TX         = 80
};

enum method {
    // Connection
    CONNECTION_START = 10,
    CONNECTION_START_OK,

    CONNECTION_TUNE = 30,
    CONNECTION_TUNE_OK,

    CONNECTION_OPEN = 40,
    CONNECTION_OPEN_OK,

    CONNECTION_CLOSE = 50,
    CONNECTION_CLOSE_OK,

    // Channel
    CHANNEL_OPEN = 10,
    CHANNEL_OPEN_OK,

    CHANNEL_CLOSE = 40,
    CHANNEL_CLOSE_OK,

    // Queue
    QUEUE_DECLARE = 10,
    QUEUE_DECLARE_OK,

    // Basic
    BASIC_CONSUME = 20,
    BASIC_CONSUME_OK,
    BASIC_PUBLISH = 40,
    BASIC_DELIVER = 60,
    BASIC_ACK     = 80
};
