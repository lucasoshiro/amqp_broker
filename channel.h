#pragma once

#include "connection_state.h"
#include <pthread.h>

typedef struct {
    int channel_id;
    pthread_t thread;
    connection_state *cs;
    char recvline[MAXLINE];
    char sendline[MAXLINE];
    char current_queue_name[MAXLINE];
} channel_state;

typedef struct {
    int last_channel;
    
} channel_pool;
