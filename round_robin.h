#pragma once

#include <pthread.h>

#include "config.h"

typedef struct round_robin_node {
    int next;
    pthread_mutex_t mutex;
} round_robin_node;

typedef struct {
    int count;
    int first;
    int last;
    pthread_mutex_t mutex;
    round_robin_node subs[MAX_CONNECTIONS];
} round_robin_scheduler;

void init_round_robin_scheduler(round_robin_scheduler *rr);

void add_subscriber(round_robin_scheduler *rr, int thread_id);
