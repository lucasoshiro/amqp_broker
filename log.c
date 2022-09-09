#include "log.h"

#include <stdio.h>

int should_log = 1;

void log_state(char *state_name, connection_state *cs) {
    if (should_log)
        printf("THREAD %p\tSTATE %s\n", (void *) cs->thread, state_name);
}

void log_message_header(char sender, amqp_message_header header, connection_state *cs) {
    if (should_log)
        printf(
            "THREAD %p\t"
            "%c\t"
            "TYPE %02x\t"
            "CHANNEL %04x\t"
            "LENGTH %08x\n",
            (void *) cs->thread, sender, header.msg_type, header.channel, header.length
            );
}

void log_method_header(char sender, amqp_method_header header, connection_state *cs) {
    if (should_log)
        printf(
            "THREAD %p\t"
            "%c\t"
            "CLASS %04x\t"
            "METHOD %04x\n",
            (void *) cs->thread, sender, header.class, header.method
            );
}

