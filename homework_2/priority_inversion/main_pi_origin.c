#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h>


static pthread_mutex_t mutex_1, mutex_2;

void task_priority_high(void *p)
{
    pthread_mutex_lock(&mutex_1);
    printf("High priority task done.\n");
    pthread_mutex_unlock(&mutex_1);
}

void task_priority_normal(void *p)
{ 
    pthread_mutex_lock(&mutex_2);
    sleep(1);
    printf("Normal priority task done.\n");
    pthread_mutex_unlock(&mutex_2);
}

void task_priority_low(void *p)
{   
    pthread_mutex_lock(&mutex_1);
    sleep(1);
    printf("Low priority task done.\n");
    pthread_mutex_unlock(&mutex_1);
    
}

static void (*TASKS[])() = {
    task_priority_high,
    task_priority_normal,
    task_priority_low,
};

int main() {
#define THREADCOUNT 3
    pthread_t thread[THREADCOUNT];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    struct sched_param param;
    pthread_mutex_init(&mutex_1, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    
    for(int i = THREADCOUNT - 1; i >= 0; i--) {
        param.sched_priority = (THREADCOUNT - i) * 10;
        pthread_attr_setschedparam(&attr, &param);
        if (pthread_create(&thread[i], &attr, (void *)TASKS[i], NULL)) {
            printf("Failed to spawn thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < THREADCOUNT; i++) {
        pthread_join(thread[i], NULL);
    }
    pthread_mutex_destroy(&mutex_1);
    pthread_mutex_destroy(&mutex_2);

    return 0;
}
