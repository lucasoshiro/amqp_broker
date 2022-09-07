/* Queue

This file defines the data structure for a queue. This is both a linked list
(https://www.ime.usp.br/~pf/algoritmos/aulas/lista.html) and a flexible array
(like this one defined in the Programming in Lua book:
https://www.lua.org/pil/28.html).

 */
#pragma once
#include <pthread.h>

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
    char name[256];
    int size;
    pthread_mutex_t mutex;
    q_node *first_node;
    q_node *last_node;
} queue;

/* Malloc a new empty queue, filling its name. */
queue *new_queue(char *name);

/* Add a string to the end of beginning of the queue. */
void q_enqueue(queue *q, char *body);

/* Pops a string from the end of the queue. If the queue is empty, return NULL. */
char *q_dequeue(queue *q);

/* Free a queue and its values. */
void free_queue(queue *q);
