#include "main.h"


/**
 * @brief create new thread (linux/windows)
 * 
 * @param thread_handler thread handler function
 * @param args thread handler function params
 */
void threadCreate(void*(*thread_handler)(void*) ,void* args){
    /* new thread create */

#ifdef _WIN32
    HANDLE thread;
    thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)thread_handler,args,0,NULL);
    if(thread == NULL){
        printf("> thread create failed. code %d\n",GetLastError());
        free(args);
    }
    CloseHandle(thread);

#else
    /* pthread args */
    pthread_t pt;
    pthread_attr_t attr;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //thread detached
    int ret = pthread_create(&pt, &attr, connectHandle, (void*)args);
    if(ret != 0){
        printf("> thread create failed. code %d\n",ret);
        free(args);
    }
#endif
    ++ __THREAD__;
    printf("thread num: %d\n",__THREAD__);
}



/**
 * @brief thread exit
 * 
 */
void threadExit(size_t time_ms){
    -- __THREAD__;
    printf("thread num: %d\n",__THREAD__);
#ifdef _WIN32
        Sleep(time_ms);
        ExitThread(0);
#else
        usleep(1000 * time_ms);
        pthread_exit(0);
#endif
    

}


int lockInit(rwlock_t* lock){
    return pthread_rwlock_init(lock,NULL);

}

int readLock(rwlock_t* lock){
    return pthread_rwlock_rdlock(lock);
}

int unlock(rwlock_t* lock){
    return pthread_rwlock_unlock(lock);

}

int writeLock(rwlock_t* lock){
    return pthread_rwlock_wrlock(lock);

}

int lockDestroy(rwlock_t* lock){
    return pthread_rwlock_destroy(lock);

}