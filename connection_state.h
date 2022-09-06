#pragma once

#include "shared.h"

#define MAXLINE 4096

typedef struct {
    int connfd;
    shared_state *ss;
    char recvline[MAXLINE];
    char sendline[MAXLINE];
} connection_state;

