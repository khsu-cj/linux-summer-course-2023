#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include "mutex_pi.h"


static mutex_t mutex_1, mutex_2;

void task_priority_high(void *p)
{
    thread_node_t *thread = (thread_node_t *)p;
    mutex_lock(&mutex_1, thread);
    printf("High priority task done.\n");
    mutex_unlock(&mutex_1, thread);
}

void task_priority_normal(void *p)
{
    thread_node_t *thread = (thread_node_t *)p;
    mutex_lock(&mutex_2, thread);
    sleep(1);
    printf("Normal priority task done.\n");
    mutex_unlock(&mutex_2, thread);
}

void task_priority_low(void *p)
{
    thread_node_t *thread = (thread_node_t *)p;
    mutex_lock(&mutex_1, thread);
    sleep(1);
    printf("Low priority task done.\n");
    mutex_unlock(&mutex_1, thread);
}

static void (*TASKS[])() = {
    task_priority_high,
    task_priority_normal,
    task_priority_low,
};

int main() {
#define THREADCOUNT 3
    thread_node_t threads[THREADCOUNT];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    mutex_init(&mutex_1);
    mutex_init(&mutex_2);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    for(int i = THREADCOUNT - 1; i >= 0; i--) {
        thread_node_init(&threads[i]);
        param.sched_priority = (THREADCOUNT - i) * 10;
        threads[i].priority = param.sched_priority;
        pthread_attr_setschedparam(&attr, &param);
        if (pthread_create(&threads[i].id, &attr, (void *)TASKS[i], &threads[i])) {
            printf("Failed to spawn thread %d\n", i);
            return 1;
        }
    }
    
    for (int i = 0; i < THREADCOUNT; i++) {
        pthread_join(threads[i].id, NULL);
    }

    return 0;
}
