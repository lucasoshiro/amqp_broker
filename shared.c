#include "shared.h"

#include <sys/mman.h>
#include <string.h>
#include "queue_pool.h"
#include "util.h"

shared_state *new_shared_state() {
    shared_state *ss = shared_malloc(sizeof(*ss));
    init_queue_pool(&ss->pool);
    bzero(&ss->channels, MAX_CHANNELS * sizeof(int));
    return ss;
}

void free_shared_state(shared_state *ss) {
    free_pool(&ss->pool);
    munmap(ss, sizeof(*ss));
}
