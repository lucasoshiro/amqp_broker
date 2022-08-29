#include "queue_pool.h"
#include <stdlib.h>

typedef struct trie_node {
    queue *q;
    struct trie_node *children[256];
} trie_node;

typedef trie_node queue_pool;

queue_pool _pool;
queue_pool *pool = &_pool;

static trie_node *_get_trie_node(trie_node *, char *);
static void _free_trie_node(trie_node *);
static queue *get_queue(char *);

static trie_node *_get_trie_node(trie_node *root, char *name) {
    trie_node *child;
    char first = *name;

    if (first == '\0') return root;

    child = root->children[(int) first];

    if (child == NULL) {
        child = malloc(sizeof(*child));
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

    free(root);
}

static queue *get_queue(char *name) {
    trie_node *node = _get_trie_node(pool, name);
    return node->q;
}

void create_queue(char *name) {
    queue *q = new_queue(name);
    trie_node *node = _get_trie_node(pool, name);

    if (node->q) free(node->q);

    node->q = q;
}

void push_to(char *name, char *body) {
    queue *q = get_queue(name);
    q_push(q, body);
}

char *pop_from(char *name) {
    queue *q = get_queue(name);    
    char *s = q_pop(q);
    return s;
}

void free_pool() {
    for (int i = 0; i < 256; i++)
        if (pool->children[i] != NULL)
            _free_trie_node(pool->children[i]);
}
