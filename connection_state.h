#pragma once

#include <pthread.h>

#include "shared.h"
#include "config.h"

#define ERROR_MSG_SIZE 128

typedef struct {
    int connfd;
    int thread_id;
    shared_state *ss;
    char recvline[MAXLINE];
    char sendline[MAXLINE];
    char current_queue_name[MAX_QUEUE_NAME];
    char error_msg[ERROR_MSG_SIZE];
} connection_state;

