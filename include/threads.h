
#ifndef RASTERLIB_THREADS_H
#define RASTERLIB_THREADS_H

#if defined(_WIN32)
    #include <windows.h>

    typedef DWORD WINAPI(*RL_ThreadFunction)(LPVOID);
    typedef HANDLE RL_Thread;
    typedef HANDLE RL_Mutex;
    typedef DWORD RL_Thread_RetValue;

    inline RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter) { return CreateThread(0, 0, thread_function, thread_parameter, 0, 0); }
    inline RL_Thread_RetValue RL_JoinThread(RL_Thread thread)                                   { return WaitForSingleObject(thread, INFINITE); }
    inline void RL_DestroyThread(RL_Thread thread)                                              { CloseHandle(thread); }

    inline RL_Mutex RL_CreateMutex()           { return CreateMutex(NULL, FALSE, NULL); }
    inline void RL_LockMutex(RL_Mutex *mutex)   { WaitForSingleObject(*mutex, INFINITE); }
    inline void RL_UnlockMutex(RL_Mutex *mutex) { ReleaseMutex(*mutex); }
    inline void RL_DestroyMutex(RL_Mutex *mutex){ CloseHandle(*mutex); }
#endif

#if defined(__linux__)
    #include <pthread.h>

    typedef int (*RL_ThreadFunction)(void*);
    typedef pthread_t RL_Thread;
    typedef pthread_mutex_t RL_Mutex;
    typedef int RL_Thread_RetValue;

    inline RL_Thread RL_CreateThread(RL_ThreadFunction thread_function, void* thread_parameter) {
        RL_Thread new_thread;
        pthread_create(&new_thread, 0, thread_function, thread_parameter);
        return new_thread;
    }
    inline RL_Thread_RetValue RL_JoinThread(RL_Thread thread) { return pthread_join(thread, NULL); }
    inline void RL_DestroyThread(RL_Thread thread)            { }

    inline RL_Mutex RL_CreateMutex() {
        RL_Mutex mutex;
        pthread_mutex_init(&mutex, 0);
        return mutex;
    }
    inline void RL_LockMutex(RL_Mutex *mutex)   { pthread_mutex_lock(mutex); }
    inline void RL_UnlockMutex(RL_Mutex *mutex) { pthread_mutex_unlock(mutex); }
    inline void RL_DestroyMutex(RL_Mutex *mutex){ pthread_mutex_destroy(mutex); }
#endif

#endif
