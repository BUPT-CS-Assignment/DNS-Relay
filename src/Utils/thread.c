#include "main.h"


/**
 * @brief create new thread (linux/windows)
 * 
 * @param thread_handler thread handler function
 * @param args thread handler function params
 */
thread_t threadCreate(void*(*thread_handler)(void*) ,void* args){
    /* new thread create */
    thread_t thread;

#ifdef _WIN32
    thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)thread_handler,args,0,NULL);
    if(thread == NULL){
        printf("> thread create failed. code %d\n",GetLastError());
        free(args);
    }

#else
    /* pthread args */
    // pthread_attr_t attr;
    // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //thread detached
    int ret = pthread_create(&thread, NULL, connectHandle, (void*)args);
    if(ret != 0){
        printf("> thread create failed. code %d\n",ret);
        free(args);
    }
#endif
    ++ __THREAD__;
    printf("> thread num: %d\n",__THREAD__);

    return thread;
}


void threadDetach(thread_t thread){

#ifdef _WIN32
    CloseHandle(thread);
#else
    pthread_detach(thread);
#endif

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



int threadJoin(thread_t thread,unsigned long* retVal){
    int ret;

#ifdef _WIN32
    ret = WaitForSingleObject(thread,INFINITE);
    if(ret == WAIT_FAILED)  return ret;
    GetExitCodeThread(thread,(LPDWORD)retVal);
#else 
    ret = pthread_join(thread,&retVal);

#endif

    return ret;

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