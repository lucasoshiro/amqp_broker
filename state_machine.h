/* State machine

This is where everything happens from the connection stablishment through its
end. Every IO event (i.e., reading the queues, receiving an AMQP message from
the client or sending an AMQP to the client) leads to change the state from one
state to another, depending on what happened.

The first state is WAIT, where the state machine waits for the protocol header.
The last states are FINISHED or FAIL, depending if the execution went well or
not.
 */

#pragma once

#include "shared.h"

/* State machine states. */
typedef enum machine_state {

    /* Connection opening and closing states */
    WAIT,
    HEADER_RECEIVED,
    WAIT_START_OK,
    START_OK_RECEIVED,
    WAIT_TUNE_OK,
    WAIT_OPEN_CONNECTION,
    OPEN_CONNECTION_RECEIVED,
    CLOSE_CONNECTION_RECEIVED,

    /* Channel opening and closing states */
    WAIT_OPEN_CHANNEL,
    OPEN_CHANNEL_RECEIVED,
    CLOSE_CHANNEL_RECEIVED,

    /* Functional states */
    WAIT_FUNCTIONAL,
    QUEUE_DECLARE_RECEIVED,

    /* Publish states */
    BASIC_PUBLISH_RECEIVED,
    WAIT_PUBLISH_CONTENT_HEADER,
    WAIT_PUBLISH_CONTENT,

    /* Consume states */
    BASIC_CONSUME_RECEIVED,
    WAIT_VALUE_DEQUEUE,
    VALUE_DEQUEUE_RECEIVED,
    WAIT_CONSUME_ACK,

    /* Finish states */
    FINISHED,
    FAIL,

    /* This is not actually a state, but a constant that its value is the number
       of state.
    */
    NUM_STATES
} machine_state;

/* This is the main function for each connection. connfd is the file descriptor
   of the socket of the connection, and ss is the shared state between the
   connections. */
void state_machine_main(int connfd, shared_state *ss);
