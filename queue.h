#pragma once

typedef struct q_node {
    struct q_node *parent;
    int length;
    char body[1];
} q_node;


typedef struct queue {
    // https://live.rabbitmq.com/queues.html#name
    char name[256];
    int size;
    q_node *last_node;
} queue;

queue *new_queue(char *name);

void q_push(queue *q, char *body);

char *q_pop(queue *q);

void free_queue(queue *q);
