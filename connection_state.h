#pragma once

#include "shared.h"
#include <pthread.h>

#define MAXLINE 4096

typedef struct {
    int connfd;
    int thread_id;
    shared_state *ss;
    char recvline[MAXLINE];
    char sendline[MAXLINE];
    char current_queue_name[MAXLINE];
} connection_state;

