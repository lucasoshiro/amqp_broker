#include "queue_pool.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "util.h"
#include <stdio.h>

static trie_node *_get_trie_node(trie_node *, char *);
static void _free_trie_node(trie_node *);
static queue *get_queue(queue_pool *, char *);

static trie_node *_get_trie_node(trie_node *root, char *name) {
    trie_node *child;
    char first = *name;

    if (first == '\0') return root;
    child = root->children[(int) first];

    if (child == NULL) {
        child = shared_malloc(sizeof(*child));
        bzero(child, sizeof(*child));
        root->children[(int) first] = child;
    }

    return _get_trie_node(child, name + 1);
}

static void _free_trie_node(trie_node *root) {
    if (root == NULL) return;

    for (int i = 0; i < 256; i++)
        _free_trie_node(root->children[i]);

    if (root->q != NULL)
        free_queue(root->q);

    munmap(root, sizeof(*root));
}

static queue *get_queue(queue_pool *pool, char *name) {
    trie_node *node = _get_trie_node(pool, name);
    return node->q;
}

void init_queue_pool(queue_pool *pool) {
    bzero(pool, sizeof(*pool));
}

void create_queue(queue_pool *pool, char *name) {
    queue *q = new_queue(name);
    trie_node *node = _get_trie_node(pool, name);

    if (node->q) free_queue(q);

    node->q = q;
}

void enqueue_to(queue_pool *pool, char *name, char *body) {
    queue *q = get_queue(pool, name);
    q_enqueue(q, body);
}

char *dequeue_from(queue_pool *pool, char *name) {
    queue *q = get_queue(pool, name);    
    char *s = q_dequeue(q);
    return s;
}

int queue_size(queue_pool *pool, char *name) {
    queue *q = get_queue(pool, name);
    return q == NULL ? -1 : q->size;
}

void free_pool(queue_pool *pool) {
    for (int i = 0; i < 256; i++)
        if (pool->children[i] != NULL)
            _free_trie_node(pool->children[i]);
}
