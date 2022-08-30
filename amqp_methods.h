#pragma once

enum msg_format {
    METHOD = 1,
    HEADER,
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
    CONNECTION_START = 10,
    CONNECTION_START_OK,

    CONNECTION_TUNE = 30,
    CONNECTION_TUNE_OK,

    CONNECTION_OPEN = 40,
    CONNECTION_OPEN_OK,

    CONNECTION_CLOSE = 50,
    CONNECTION_CLOSE_OK,

    CHANNEL_OPEN = 10,
    CHANNEL_OPEN_OK,

    CHANNEL_CLOSE = 40,
    CHANNEL_CLOSE_OK
};
