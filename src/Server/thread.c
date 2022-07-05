#include <main.h>


/**
 * @brief create new thread (linux/windows)
 * 
 * @param thread_handler thread handler function
 * @param args thread handler function params
 */
void threadCreate(void*(*thread_handler)(void*) ,thread_args* args){
    if(args == NULL)   return;
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

}



/**
 * @brief thread exit
 * 
 */
void threadExit(){

#ifdef _WIN32
        Sleep(10);
        //ExitThread(0);
#else
        usleep(10000);
        //pthread_exit(0);
#endif


}