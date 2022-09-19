/* Queue

This file defines the data structure for a queue. This is both a linked list
(https://www.ime.usp.br/~pf/algoritmos/aulas/lista.html) and a flexible array
(like this one defined in the Programming in Lua book:
https://www.lua.org/pil/28.html).

 */
#pragma once

#include <pthread.h>

#include "config.h"
#include "round_robin.h"

/* This is the node of a linked list, pointing to its parent. This node holds a
   variable-length string (body).

   This is a flexible array, which means that it should be allocated with
   malloc(sizeof(q_node) + length * sizeof(char)), then the last field body
   will have the exact size of the length of its content + the null terminator.
 */
typedef struct q_node {
    struct q_node *parent;
    int length;
    char body[1];
} q_node;


/* The queue, containing its name, size, a pointer to the first node and to the
   last node.
*/
typedef struct queue {
    char name[MAX_QUEUE_NAME];
    int size;
    round_robin_scheduler rr;
    pthread_mutex_t mutex;
    pthread_mutex_t cond_mutex;
    pthread_cond_t cond;
    q_node *first_node;
    q_node *last_node;
} queue;

/* Malloc a new empty queue, filling its name. */
queue *new_queue(char *name);

/* Add a string to the end of beginning of the queue. */
void q_enqueue(queue *q, char *body);

/* Wait until the round of the thread, then pop the last value of the queue to
   the dest string. */
char *q_dequeue_rr(queue *q, int thread_id, char *dest);

/* Free a queue and its values. */
void free_queue(queue *q);

/* Size of a queue. */
int q_size(queue *q);

/* Subscribe the provided thread to a queue. */
void q_add_subscriber(queue *q, int thread_id);
