/* Round robin

   This defines the round robin scheduling for a queue.
 */

#pragma once

#include <pthread.h>

#include "config.h"

/* A round robin node, that represents a thread in a scheduler. The round robin
   nodes holds a cond variable and its mutex, and the thread_id of the current,
   the previous and the next thread.

   This way, this is kind of a linked list, but this does not hold a reference
   to the next node. This holds the thread_id of the next thread instead. The
   thread_id is enough to index another node.

   The conditional variable is used to wake a thread when it's its round. The
   previous thread sends a signal to this, which will be unlocked.
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

/* Remove a subscriber from a scheduler. */
void remove_subscriber(round_robin_scheduler *rr, int thread_id);

/* Changes the internal state of the schedule to the next subscriber. The thread
   of the next subscriber will be signalized, and then it will proceed the
   dequeue.
*/
int next_thread(round_robin_scheduler *rr);

/* Wait until it's the round of thread with id thread_id. */
void wait_round(round_robin_scheduler *rr, int thread_id);
