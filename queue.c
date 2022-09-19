#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "util.h"

#include <stdio.h>

static char *q_dequeue(queue *q, char *dest);

queue *new_queue(char *name) {
    queue *q;
    q = malloc(sizeof(*q));

    strcpy(q->name, name);

    init_round_robin_scheduler(&q->rr);

    q->first_node = NULL;
    q->last_node = NULL;
    q->size = 0;

    pthread_mutex_init(&q->mutex, NULL);
    pthread_mutex_init(&q->new_msg_mutex, NULL);
    pthread_cond_init(&q->new_msg_cond, NULL);
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

    /* Tell subscribers that there's a new messsage. */
    pthread_mutex_lock(&q->new_msg_mutex);
    pthread_cond_broadcast(&q->new_msg_cond);
    pthread_mutex_unlock(&q->new_msg_mutex);

    pthread_mutex_unlock(&q->mutex);
}

static char *q_dequeue(queue *q, char *dest) {
    q_node *last;
    char *ret = NULL;

    /* Wait until there's a message */
    pthread_mutex_lock(&q->mutex);

    last = q->last_node;
    if (last) {
        ret = dest;
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

char *q_dequeue_rr(queue *q, int thread_id, char *dest) {
    char *ret;
    round_robin_scheduler *rr = &q->rr;
    round_robin_node *node = &rr->subs[thread_id];
    round_robin_node *next;

    /* Wait until it's the round of this thread */
    printf("WAITING ROUND %d CURRENT %d\n", thread_id, rr->current);

    if (rr->current != thread_id) {
        pthread_mutex_lock(&node->round_mutex);
        printf("WAITING ROUND COND %p\n", &node->round_cond);
        pthread_cond_wait(&node->round_cond, &node->round_mutex);
        printf("SIGNAL RECEIVED %d\n", thread_id);
        pthread_mutex_unlock(&node->round_mutex);
    }

    printf("WAITING MSG %d\n", thread_id);

    if (q->size == 0) {
        pthread_mutex_lock(&q->new_msg_mutex);
        pthread_cond_wait(&q->new_msg_cond, &q->new_msg_mutex);
        pthread_mutex_unlock(&q->new_msg_mutex);
    }

    ret = q_dequeue(q, dest);
    next_thread(&q->rr);

    /* Tell next that a message was dequeued. */
    next = &rr->subs[node->next];
    pthread_mutex_lock(&next->round_mutex);
    printf("SIGNALING %p\n", &next->round_cond);
    pthread_cond_broadcast(&next->round_cond);
    pthread_mutex_unlock(&next->round_mutex);
    printf("SIGNAL SENT %d TO %d\n", thread_id, node->next);

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

int q_size(queue *q) {
    int size;
    pthread_mutex_lock(&q->mutex);
    size = q->size;
    pthread_mutex_unlock(&q->mutex);

    return size;
}

void q_add_subscriber(queue *q, int thread_id) {
    add_subscriber(&q->rr, thread_id);
}
