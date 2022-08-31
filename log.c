#include "log.h"

#include <stdio.h>

int should_log = 1;

void log_state(char *state_name) {
    if (should_log)
        printf("STATE: %s\n", state_name);
}

void log_message_header(char sender, amqp_message_header header) {
    if (should_log)
        printf(
            "%c: "
            "type: %02x, "
            "channel: %04x, "
            "length: %08x\n",
            sender, header.msg_type, header.channel, header.length
            );
}

void log_method_header(char sender, amqp_method_header header) {
    if (should_log)
        printf(
            "%c: "
            "class: %04x, "
            "method: %04x\n",
            sender, header.class, header.method
            );
}

