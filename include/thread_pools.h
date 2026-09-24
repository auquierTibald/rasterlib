#ifndef RASTERLIB_THREAD_POOLS_H
#define RASTERLIB_THREAD_POOLS_H

#include <stdbool.h>

#include <rasterlib.h>
#include <typedefs.h>
#include <threads.h>
#include <utils-da.h>

typedef struct {
    RL_Triangle *tri;
    int minx, miny, maxx, maxy;
} RL_Task;

typedef void (*task_func)(RL_Context *context, RL_Task task);

typedef da(RL_Task) RL_TaskQueue;

typedef struct thread_arg thread_arg;

typedef struct {
    RL_Context *context;
    RL_Mutex mutex;
    RL_Thread *threads;
    RL_TaskQueue *queues;
    size_t n_threads;
    task_func execute_task;
    thread_arg *args;
    bool active;
} RL_ThreadPool;

typedef struct thread_arg{
    RL_ThreadPool *pool;
    size_t thread;
} thread_arg;

RL_TaskQueue RL_CreateTaskQueue(void);
void RL_DestroyTaskQueue(RL_TaskQueue *queue);

RL_ThreadPool *RL_CreateThreadPool(RL_Context *context, int n_threads, task_func execute_task);
void RL_AddTask(const RL_ThreadPool *pool, int thread, RL_Task task);
void RL_DestroyThreadPool(RL_ThreadPool *pool);


#endif //RASTERLIB_THREAD_POOLS_H
