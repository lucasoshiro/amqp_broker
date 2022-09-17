/* Queue pool

   This file defines the queue pool, that contains all the queues, and the
   functions that manipulate it.

   The queue pool is a trie (https://pt.wikipedia.org/wiki/Trie), a tree data
   structure that acts as a symbol table and makes it easy to use strings as
   keys. The strings here are the name of the queue. The values are the queues.

 */
#pragma once
#include "queue.h"
#include <pthread.h>

/* A trie node, containing a queue and pointers to the 256 children. */
typedef struct trie_node {
    queue *q;
    pthread_mutex_t mutex;
    struct trie_node *children[256];
} trie_node;

/* A queue pool is just the root of the trie. */
typedef trie_node queue_pool;

/* Initialize the queue pool. */
void init_queue_pool(queue_pool *pool);

/* Create a new empty queue in a queue pool. */
void create_queue(queue_pool *pool, char *name);

/* Return a queue, given its name */
queue *get_queue(queue_pool *pool, char *name);

/* Add a value to a queue in a queue pool. */
void enqueue_to(queue_pool *pool, char *name, char *body);

/* Pop a value from a queue in a queue pool. */
char *dequeue_from(queue_pool *pool, char *name);

/* Return the size of a queue in a queue pool. */
int queue_size(queue_pool *pool, char *name);

/* Free a queue pool */
void free_pool(queue_pool *pool);
