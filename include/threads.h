
#ifndef RASTERLIB_THREADS_H
#define RASTERLIB_THREADS_H

#if defined(_WIN32)
    #include <windows.h>

    typedef DWORD WINAPI(*RL_ThreadFunction)(LPVOID);
    typedef HANDLE RL_Thread;
    typedef HANDLE RL_Mutex;
    typedef DWORD RL_Thread_RetValue;

     RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter);
     RL_Thread_RetValue RL_JoinThread(RL_Thread thread);
     void RL_DestroyThread(RL_Thread thread);

     RL_Mutex RL_CreateMutex();
     void RL_LockMutex(RL_Mutex *mutex);
     void RL_UnlockMutex(RL_Mutex *mutex);
     void RL_DestroyMutex(RL_Mutex *mutex);
#endif

#if defined(__linux__)
    #include <pthread.h>

    typedef void* (*RL_ThreadFunction)(void*);
    typedef pthread_t RL_Thread;
    typedef pthread_mutex_t RL_Mutex;
    typedef int RL_Thread_RetValue;

     RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter);
     RL_Thread_RetValue RL_JoinThread(RL_Thread thread);
     void RL_DestroyThread(RL_Thread thread);

     RL_Mutex RL_CreateMutex();
     void RL_LockMutex(RL_Mutex *mutex);
     void RL_UnlockMutex(RL_Mutex *mutex);
     void RL_DestroyMutex(RL_Mutex *mutex);
#endif

#endif
