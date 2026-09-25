#include "thread_pools.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils-da.h"


static void *task_pool_func(void *args) {
    thread_arg *arg = args;
    RL_ThreadPool *pool = arg->pool;
    RL_TaskQueue *queue = &pool->queues[arg->thread];
    printf("thread %lu launched\n", arg->thread);
    while (pool->active) {
        RL_LockMutex(&pool->mutex);
	//printf("queue size  inside thread %d : %lu\n", arg->thread, queue->size);
        if (queue->size > 0) {
            const RL_Task task = queue->data[--queue->size];
            pool->execute_task(pool->context, task);
            //printf("executed task : %d %d %d %d\n", task.minx, task.miny, task.maxx, task.maxy);
        }
        RL_UnlockMutex(&pool->mutex);
    }
    return NULL;
}

RL_TaskQueue RL_CreateTaskQueue(void) {
    return (RL_TaskQueue)da_alloc(RL_Task, 2);
}
void RL_DestroyTaskQueue(RL_TaskQueue *queue) {
    da_free(queue);
}

RL_ThreadPool *RL_CreateThreadPool(RL_Context *context, int n_threads, task_func execute_task) {
    RL_ThreadPool *pool = calloc(1, sizeof(RL_ThreadPool));

    *pool = (RL_ThreadPool){
        .context = context,
        .execute_task = execute_task,
        .n_threads = n_threads,
        .mutex = RL_CreateMutex(),
        .threads = malloc(n_threads * sizeof(RL_Thread)),
        .queues = malloc(n_threads * sizeof(RL_TaskQueue)),
        .args = malloc(n_threads * sizeof(thread_arg)),
	    .active = true
    };

    for (size_t i = 0; i < pool->n_threads; i++) {
        pool->queues[i] = RL_CreateTaskQueue();
        pool->args[i] = (thread_arg){.pool = pool, .thread = i};
        pool->threads[i] = RL_CreateThread(task_pool_func, &pool->args[i]);
    }

    return pool;
}

void RL_AddTask(const RL_ThreadPool *pool, const int thread, const RL_Task task) {
    da_append(&pool->queues[thread], RL_Task, task);
}

void RL_DestroyThreadPool(RL_ThreadPool *pool) {
    pool->active = false;
    for (size_t i = 0; i < pool->n_threads; i++) {
        RL_JoinThread(pool->threads[i]);
        RL_DestroyThread(pool->threads[i]);
        RL_DestroyTaskQueue(&pool->queues[i]);
    }
    RL_DestroyMutex(&pool->mutex);
    free(pool->threads);
    free(pool->args);
    free(pool->queues);
    free(pool);
}
