#pragma once
#include "queue.h"

void create_queue(char *name);

void enqueue_to(char *name, char *body);

char *dequeue_from(char *name);

void free_pool();
