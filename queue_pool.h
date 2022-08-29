#pragma once
#include "queue.h"

void create_queue(char *name);

void push_to(char *name, char *body);

char *pop_from(char *name);

void free_pool();
