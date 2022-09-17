#include "round_robin.h"
#include <stdio.h>

void init_round_robin_scheduler(round_robin_scheduler *rr) {
    rr->count = 0;
    rr->last = -1;
    rr->first = -1;

    pthread_mutex_init(&rr->mutex, NULL);
}

void add_subscriber(round_robin_scheduler *rr, int thread_id) {
    round_robin_node *node;

    pthread_mutex_lock(&rr->mutex);

    node = &rr->subs[thread_id];

    pthread_mutex_init(&node->mutex, NULL);
    pthread_mutex_lock(&node->mutex);

    if (rr->count == 0) {
        rr->first = thread_id;

        /* Unlocking this subscribe, as it is the only one */
        pthread_mutex_unlock(&node->mutex);
    }
    else {
        round_robin_node *last = &rr->subs[rr->last];
        last->next = thread_id;
    }

    rr->last = thread_id;
    node->next = rr->first;

    (rr->count)++;
    pthread_mutex_unlock(&rr->mutex);
}

