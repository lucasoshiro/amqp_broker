#include "log.h"

#include <stdio.h>
#include <pthread.h>

int should_log = 1;

pthread_mutex_t log_mutex;

void log_state(char *state_name, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf("THREAD %d\tSTATE\t%s\n", cs->thread_id, state_name);
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_message_header(char sender, amqp_message_header header, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %d\t"
            "%c\t"
            "TYPE %02x\t"
            "CHANNEL %04x\t"
            "LENGTH %08x\n",
            cs->thread_id, sender, header.msg_type, header.channel, header.length
            );
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_method_header(char sender, amqp_method_header header, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %d\t"
            "%c\t"
            "CLASS %04x\t"
            "METHOD %04x\n",
            cs->thread_id, sender, header.class, header.method
            );
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_queue_creation(char *queue_name, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %d\t"
            "DECLARE %s\n",
            cs->thread_id, queue_name
            );
        pthread_mutex_unlock(&log_mutex);
    }
}
