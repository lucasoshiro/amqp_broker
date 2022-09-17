#include "queue_pool.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"

static trie_node *_get_trie_node(trie_node *, char *);
static void _free_trie_node(trie_node *);

static trie_node *_get_trie_node(trie_node *root, char *name) {
    trie_node *child;
    unsigned char first = *name;

    if (first == '\0') return root;
    child = root->children[first];

    if (child == NULL) {
        child = malloc(sizeof(*child));
        bzero(child, sizeof(*child));
        root->children[first] = child;
    }

    return _get_trie_node(child, name + 1);
}

static void _free_trie_node(trie_node *root) {
    if (root == NULL) return;

    for (int i = 0; i < 256; i++)
        _free_trie_node(root->children[i]);

    if (root->q != NULL)
        free_queue(root->q);

    free(root);
}

void init_queue_pool(queue_pool *pool) {
    bzero(pool, sizeof(*pool));
}

void create_queue(queue_pool *pool, char *name) {
    trie_node *node;

    pthread_mutex_lock(&pool->mutex);
    node = _get_trie_node(pool, name);
    pthread_mutex_unlock(&pool->mutex);

    pthread_mutex_lock(&node->mutex);
    if (node->q == NULL) node->q = new_queue(name);
    pthread_mutex_unlock(&node->mutex);
}

queue *get_queue(queue_pool *pool, char *name) {
    trie_node *node = _get_trie_node(pool, name);
    return node->q;
}

void enqueue_to(queue_pool *pool, char *name, char *body) {
    queue *q;

    pthread_mutex_lock(&pool->mutex);
    q = get_queue(pool, name);
    pthread_mutex_unlock(&pool->mutex);

    q_enqueue(q, body);
}

char *dequeue_from(queue_pool *pool, char *name) {
    queue *q;
    pthread_mutex_lock(&pool->mutex);
    q = get_queue(pool, name);    
    pthread_mutex_unlock(&pool->mutex);

    return q_dequeue(q);
}

int queue_size(queue_pool *pool, char *name) {
    queue *q;
    pthread_mutex_lock(&pool->mutex);
    q = get_queue(pool, name);
    pthread_mutex_unlock(&pool->mutex);

    return q == NULL ? -1 : q_size(q);
}

void free_pool(queue_pool *pool) {
    for (int i = 0; i < 256; i++)
        if (pool->children[i] != NULL)
            _free_trie_node(pool->children[i]);
}
