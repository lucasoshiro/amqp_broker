#pragma once

#include "shared.h"
#include <pthread.h>

#define MAXLINE 131073
#define MAX_QUEUE_NAME 256
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

