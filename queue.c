#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "util.h"

queue *new_queue(char *name) {
    queue *q;
    q = malloc(sizeof(*q));

    strcpy(q->name, name);
    q->first_node = NULL;
    q->last_node = NULL;
    q->size = 0;
    return q;
}

void q_enqueue(queue *q, char *body) {
    q_node *n;
    int length = strlen(body);

    n = malloc(sizeof(*n) + length * sizeof(char));
    n->parent = NULL;
    n->length = length;
    strcpy(n->body, body);
    
    pthread_mutex_lock(&q->mutex);

    if (q->first_node != NULL)
        q->first_node->parent = n;

    q->first_node = n;

    if (q->last_node == NULL)
        q->last_node = n;
    q->size += 1;

    pthread_mutex_unlock(&q->mutex);
}

char *q_dequeue(queue *q) {
    q_node *last;
    char *ret = NULL;

    pthread_mutex_lock(&q->mutex);
    last = q->last_node;
    if (last) {
        int length = last->length;
        ret = malloc(sizeof(char) * (1 + length));
        strcpy(ret, last->body);
        q->last_node = last->parent;
        free(last);

        if (q->first_node == last)
            q->first_node = NULL;

        q->size -= 1;
    }

    pthread_mutex_unlock(&q->mutex);
    return ret;
}

void free_queue(queue *q) {
    for (q_node *n = q->last_node; n != NULL;) {
        q_node *old = n;
        n = old->parent;
        free(old);
    }

    free(q);
}
