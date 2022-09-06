#pragma once

#include "queue_pool.h"

#define MAX_CHANNELS 256

typedef struct {
    queue_pool pool;
    int channels[MAX_CHANNELS];
} shared_state;

void init_shared_state(shared_state *ss);
