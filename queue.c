#include <stdlib.h>
#include <string.h>
#include "queue.h"

queue *new_queue(char *name) {
    queue *q;
    q = malloc(sizeof(*q));

    strcpy(q->name, name);
    q->last_node = NULL;
    q->size = 0;
    return q;
}

void q_push(queue *q, char *body) {
    q_node *n;
    int length = strlen(body);

    n = malloc(sizeof(*n) + length * sizeof(char));
    n->parent = q->last_node;
    n->length = length;
    strcpy(n->body, body);
    
    q->last_node = n;
    q->size += 1;
}

char *q_pop(queue *q) {
    q_node *last = q->last_node;
    char *ret = NULL;

    if (last) {
        int length = last->length;
        ret = malloc(sizeof(char) * (1 + length));
        strcpy(ret, last->body);
        q->last_node = last->parent;
        free(last);

        q->size -= 1;
    }

    return ret;
}

void free_queue(queue *q) {
    for (q_node *n = q->last_node; n != NULL;) {
        n = n->parent;
        free(n);
    }

    free(q);
}
