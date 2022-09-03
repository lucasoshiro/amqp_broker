#pragma once
#include "queue.h"

typedef struct trie_node {
    queue *q;
    struct trie_node *children[256];
} trie_node;

typedef trie_node queue_pool;

void init_queue_pool(queue_pool *pool);

void create_queue(queue_pool *pool, char *name);

void enqueue_to(queue_pool *pool, char *name, char *body);

char *dequeue_from(queue_pool *pool, char *name);

int queue_size(queue_pool *pool, char *name);

void free_pool(queue_pool *pool);
