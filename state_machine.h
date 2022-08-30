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
    FINISHED,
    FAIL,
    NUM_STATES
} machine_state;

void state_machine_main(int _connfd);
