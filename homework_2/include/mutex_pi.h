#pragma once

#include <stdbool.h>
#include "atomic.h"
#include "futex.h"
#include "thread_node.h"

typedef struct {
    atomic int  state;
    pthread_t   owner;
    spinlock_t  spinlock;
} mutex_t;

enum {
    MUTEX_LOCKED = 1 << 0,
    MUTEX_SLEEPING = 1 << 1,
};


static inline void mutex_init(mutex_t *mutex)
{
    atomic_init(&mutex->state, 0);
    mutex->owner = 0;
    spin_init(&mutex->spinlock);
}

static bool mutex_trylock(mutex_t *mutex)
{
    int state = load(&mutex->state, relaxed);
    if (state & MUTEX_LOCKED)
        return false;

    state = fetch_or(&mutex->state, MUTEX_LOCKED, relaxed);
    if (state & MUTEX_LOCKED)
        return false;

    thread_fence(&mutex->state, acquire);
    return true;
}

static inline void mutex_lock(mutex_t *mutex, thread_node_t *thread)
{
#define MUTEX_SPINS 128
    for (int i = 0; i < MUTEX_SPINS; ++i) {
        if (mutex_trylock(mutex)) {
            /* Update the owner of this mutex to current thread */
            spin_lock(&mutex->spinlock);
            mutex->owner = thread->id;
            spin_unlock(&mutex->spinlock);
            return;
        }
        spin_hint();
    }

    int state = exchange(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING, relaxed);

    while (state & MUTEX_LOCKED) {
        int policy_owner, policy_curr;
        struct sched_param param_owner, param_curr;

        /* 
         * Try to promote the priority of the thread who is owning this mutex
         * if the priority of current thread is higher than it.
         */
        spin_lock(&thread->spinlock);
        pthread_getschedparam(mutex->owner, &policy_owner, &param_owner);
        pthread_getschedparam(thread->id, &policy_curr, &param_curr);
        if (param_owner.sched_priority < param_curr.sched_priority) {
            param_owner.sched_priority = param_curr.sched_priority;
            pthread_setschedparam(mutex->owner, policy_owner, &param_owner);
        }
        spin_unlock(&thread->spinlock);

        futex_wait(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING);
        state = exchange(&mutex->state, MUTEX_LOCKED | MUTEX_SLEEPING, relaxed);
    }

    /* Update the owner of this mutex to current thread */
    spin_lock(&mutex->spinlock);
    mutex->owner = thread->id;
    spin_unlock(&mutex->spinlock);

    thread_fence(&mutex->state, acquire);
}

static inline void mutex_unlock(mutex_t *mutex, thread_node_t *thread)
{
    int policy;
    struct sched_param param;

    int state = exchange(&mutex->state, 0, release);

    /* Clear the owner of this mutex */
    spin_lock(&mutex->spinlock);
    mutex->owner = 0;
    spin_unlock(&mutex->spinlock);

    /* Restore priority */
    spin_lock(&thread->spinlock);
    pthread_getschedparam(thread->id, &policy, &param);
    if (param.sched_priority != thread->priority) {
        param.sched_priority = thread->priority;
        pthread_setschedparam(mutex->owner, policy, &param);
    }
    spin_unlock(&thread->spinlock);

    if (state & MUTEX_SLEEPING)
        futex_wake(&mutex->state, 1);
}
