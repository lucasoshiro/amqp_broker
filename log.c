#include "log.h"

#include <stdio.h>
#include <pthread.h>

int should_log = 1;

pthread_mutex_t log_mutex;

void log_state(char *state_name, connection_state *cs) {
    if (!should_log) return;

    pthread_mutex_lock(&log_mutex);
    printf("THREAD %d\tSTATE\t%s\n", cs->thread_id, state_name);
    pthread_mutex_unlock(&log_mutex);
}

void log_message_header(char sender, amqp_message_header header, connection_state *cs) {
    if (!should_log) return;

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

void log_method_header(char sender, amqp_method_header header, connection_state *cs) {
    if (!should_log) return;

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

void log_queue_creation(char *queue_name, connection_state *cs) {
    if (!should_log) return;

    pthread_mutex_lock(&log_mutex);
    printf(
        "THREAD %d\t"
        "DECLARE %s\n",
        cs->thread_id, queue_name
        );
    pthread_mutex_unlock(&log_mutex);
}

void log_connection_accept(int thread_id, int connfd) {
    if (!should_log) return;

    pthread_mutex_lock(&log_mutex);
    printf(
        "THREAD %d\t"
        "ACCEPT FD %d\n",
        thread_id, connfd
        );
    pthread_mutex_unlock(&log_mutex);
}

void log_connection_close(int thread_id, int connfd) {
    if (!should_log) return;

    pthread_mutex_lock(&log_mutex);
    printf(
        "THREAD %d\t"
        "CLOSE FD %d\n",
        thread_id, connfd
        );
    pthread_mutex_unlock(&log_mutex);
}

void log_max_thread_reached() {
    if (!should_log) return;
    
    pthread_mutex_lock(&log_mutex);
    printf(
        "MAIN\t"
        "MAX THREAD REACHED\n"
        );
    pthread_mutex_unlock(&log_mutex);
}

void log_fail(connection_state *cs) {
    if (!should_log) return;
    pthread_mutex_lock(&log_mutex);
    printf(
        "THREAD %d\t"
        "FAIL!\t"
        "MSG: %s\n",
        cs->thread_id,
        cs->error_msg
        );
    pthread_mutex_unlock(&log_mutex);
}

void log_finished(connection_state *cs) {
    if (!should_log) return;
    pthread_mutex_lock(&log_mutex);
    printf(
        "THREAD %d\t"
        "FINISHED!\n",
        cs->thread_id
        );
    pthread_mutex_unlock(&log_mutex);
}
