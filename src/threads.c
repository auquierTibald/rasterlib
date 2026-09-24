#include "threads.h"

#if defined(_WIN32)
    #include <windows.h>

     RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter) { return CreateThread(0, 0, thread_function, thread_parameter, 0, 0); }
     RL_Thread_RetValue RL_JoinThread(RL_Thread thread)                                   { return WaitForSingleObject(thread, INFINITE); }
     void RL_DestroyThread(RL_Thread thread)                                              { CloseHandle(thread); }

     RL_Mutex RL_CreateMutex()           { return CreateMutex(NULL, FALSE, NULL); }
     void RL_LockMutex(RL_Mutex *mutex)   { WaitForSingleObject(*mutex, INFINITE); }
     void RL_UnlockMutex(RL_Mutex *mutex) { ReleaseMutex(*mutex); }
     void RL_DestroyMutex(RL_Mutex *mutex){ CloseHandle(*mutex); }
#endif

#if defined(__linux__)
    #include <pthread.h>

     RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter) {
        RL_Thread new_thread;
        pthread_create(&new_thread, 0, thread_function, thread_parameter);
        return new_thread;
    }
     RL_Thread_RetValue RL_JoinThread(RL_Thread thread) { return pthread_join(thread, NULL); }
     void RL_DestroyThread(RL_Thread thread)            { }

     RL_Mutex RL_CreateMutex() {
        RL_Mutex mutex;
        pthread_mutex_init(&mutex, 0);
        return mutex;
    }
     void RL_LockMutex(RL_Mutex *mutex)   { pthread_mutex_lock(mutex); }
     void RL_UnlockMutex(RL_Mutex *mutex) { pthread_mutex_unlock(mutex); }
     void RL_DestroyMutex(RL_Mutex *mutex){ pthread_mutex_destroy(mutex); }
#endif
