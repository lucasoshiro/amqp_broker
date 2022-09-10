#include "log.h"

#include <stdio.h>
#include <pthread.h>

int should_log = 1;

pthread_mutex_t log_mutex;

void log_state(char *state_name, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf("THREAD %p\tSTATE %s\n", (void *) cs->thread, state_name);
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_message_header(char sender, amqp_message_header header, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %p\t"
            "%c\t"
            "TYPE %02x\t"
            "CHANNEL %04x\t"
            "LENGTH %08x\n",
            (void *) cs->thread, sender, header.msg_type, header.channel, header.length
            );
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_method_header(char sender, amqp_method_header header, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %p\t"
            "%c\t"
            "CLASS %04x\t"
            "METHOD %04x\n",
            (void *) cs->thread, sender, header.class, header.method
            );
        pthread_mutex_unlock(&log_mutex);
    }
}

void log_queue_creation(char *queue_name, connection_state *cs) {
    if (should_log) {
        pthread_mutex_lock(&log_mutex);
        printf(
            "THREAD %p\t"
            "DECLARE %s\n",
            (void *) cs->thread, queue_name
            );
        pthread_mutex_unlock(&log_mutex);
    }
}
