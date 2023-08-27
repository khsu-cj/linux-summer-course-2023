#pragma once

#include "spinlock.h"


typedef struct {
    pthread_t id;
    spinlock_t spinlock;
    int priority;
} thread_node_t;

static inline void thread_node_init(thread_node_t *node)
{
    node->id = 0;
    spin_init(&node->spinlock);
    node->priority = 0;
}
