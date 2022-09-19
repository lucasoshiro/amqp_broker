#include "round_robin.h"

#include <stdio.h>

void init_round_robin_scheduler(round_robin_scheduler *rr) {
    rr->count = 0;
    rr->last = -1;
    rr->first = -1;
    rr->current = -1;
    
    pthread_mutex_init(&rr->mutex, NULL);
}

void add_subscriber(round_robin_scheduler *rr, int thread_id) {
    round_robin_node *node;

    pthread_mutex_lock(&rr->mutex);

    node = &rr->subs[thread_id];

    if (rr->count == 0) {
        rr->first = thread_id;
        rr->last = thread_id;
        rr->current = thread_id;
    }
    else {
        round_robin_node *last = &rr->subs[rr->last];
        round_robin_node *first = &rr->subs[rr->first];
        last->next = thread_id;
        first->prev = thread_id;
    }

    node->prev = rr->last;
    node->next = rr->first;
    node->thread_id = thread_id;

    pthread_mutex_init(&node->round_mutex, NULL);
    pthread_cond_init(&node->round_cond, NULL);

    rr->last = thread_id;

    (rr->count)++;
    pthread_mutex_unlock(&rr->mutex);
}

void remove_subscriber(round_robin_scheduler *rr, int thread_id) {
    round_robin_node *node, *prev, *next;
    int prev_id, next_id;

    node = &rr->subs[thread_id];

    pthread_mutex_lock(&rr->mutex);

    prev_id = node->prev;
    next_id = node->next;

    prev = &rr->subs[prev_id];
    next = &rr->subs[next_id];

    prev->next = next_id;
    next->prev = prev_id;
    
    if (rr->current == thread_id)
        rr->current = next_id;

    if (rr->first == thread_id)
        rr->first = next_id;

    if (rr->last == thread_id)
        rr->last = prev_id;

    (rr->count)--;

    if (rr->count == 0) {
        rr->first = rr->last = -1;
    }

    pthread_mutex_unlock(&rr->mutex);
}

int next_thread(round_robin_scheduler *rr) {
    int current, next;

    pthread_mutex_lock(&rr->mutex);

    current = rr->current;
    next = rr->subs[current].next;

    rr->current = next;

    printf("CHANGING %d TO %d\n", current, next);

    pthread_mutex_unlock(&rr->mutex);

    return next;
}
