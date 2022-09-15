/* Shared data

   This file defines shared_state, a struct that holds cross-connection data.
 */
#pragma once

#include "queue_pool.h"

typedef struct {
    queue_pool q_pool;
} shared_state;

/* Initialize the shared stated pointed by ss. */
void init_shared_state(shared_state *ss);
