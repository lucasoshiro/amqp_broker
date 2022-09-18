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
        rr->last = thread_id;

        /* Unlocking this subscribe, as it is the only one */
        pthread_mutex_unlock(&node->mutex);
    }
    else {
        round_robin_node *last = &rr->subs[rr->last];
        round_robin_node *first = &rr->subs[rr->first];
        last->next = thread_id;
        first->prev = thread_id;
    }

    node->prev = rr->last;
    node->next = rr->first;
    rr->last = thread_id;

    (rr->count)++;
    pthread_mutex_unlock(&rr->mutex);
}

void remove_subscriber(round_robin_scheduler *rr, int thread_id) {
    round_robin_node *node, *prev, *next;
    int prev_id, next_id;

    node = &rr->subs[thread_id];

    /* Wait until someone unlocks this. */
    pthread_mutex_lock(&node->mutex);

    printf("REMOVING %d %d %d!!!!\n", thread_id, node->prev, node->next);

    pthread_mutex_lock(&rr->mutex);

    prev_id = node->prev;
    next_id = node->next;

    prev = &rr->subs[prev_id];
    next = &rr->subs[next_id];

    prev->next = next_id;
    next->prev = prev_id;
    
    if (rr->first == thread_id)
        rr->first = next_id;

    if (rr->last == thread_id)
        rr->last = prev_id;

    (rr->count)--;

    if (rr->count == 0) {
        rr->first = rr->last = -1;
    }

    printf("UNLOCKING %d!!!!\n", next_id);
    pthread_mutex_unlock(&next->mutex);
    printf("BYE!!!!\n");
    pthread_mutex_unlock(&rr->mutex);
}
