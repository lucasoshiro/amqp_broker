/* Round robin

   This defines the round robin scheduling for a queue.
 */

#pragma once

#include <pthread.h>

#include "config.h"

/* A round robin node, that represents a thread in a schueduler. The round
   robin nodes holds a mutex and the thread_id of the previous and the next
   thread.

   This way, this is kind of a linked list, but this does not hold a reference
   to the next node. This holds the thread_id of the next thread instead. The
   thread_id is enough to index another node.

   The mutex is locked when it's the time for the thread represented by this
   node to get a value from the queue. This can only be unlocked by the previous
   node.
*/
typedef struct round_robin_node {
    int prev;
    int next;
    int thread_id;
    pthread_mutex_t round_mutex;
    pthread_cond_t round_cond;
} round_robin_node;

/* The round robin scheduler. This holds a list of round robin nodes. For each
   thread subscribed in the container queue, the list contains a round robin
   node, indexed by the thread_id of the thread.
*/
typedef struct {
    int count;
    int first;
    int last;
    int current;
    pthread_mutex_t mutex;
    round_robin_node subs[MAX_CONNECTIONS];
} round_robin_scheduler;

/* Initialize the round robin scheduler pointed by rr. */
void init_round_robin_scheduler(round_robin_scheduler *rr);

/* Add a subscriber to a scheduler. */
void add_subscriber(round_robin_scheduler *rr, int thread_id);

void remove_subscriber(round_robin_scheduler *rr, int thread_id);

int next_thread(round_robin_scheduler *rr);
