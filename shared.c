#include "shared.h"

#include <string.h>
#include "queue_pool.h"
#include "util.h"

void init_shared_state(shared_state *ss) {
    init_queue_pool(&ss->pool);
    bzero(&ss->channels, MAX_CHANNELS * sizeof(int));
}
