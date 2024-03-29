#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"Ws2_32.lib")

#define ERROR_CODE GetLastError()

#else

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

typedef int SOCKET;

#define SOCKET_ERROR -1
#define ERROR_CODE errno

#endif

typedef pthread_t thread_t;

/* Debug setting */
#define DEBUG_L0    0
#define DEBUG_L1    1
#define DEBUG_L2    2
extern int          __DEBUG__;
extern int          __THREAD__;


/* inet change */
int inetParse(int af, void* url_src, char* dest);
int inetFormat(int af, char* url_src, void* dest);


/* Basic Socket Struct Contents FD & ADDR */
typedef struct Socket
{
    SOCKET _fd;
    struct sockaddr_in _addr;

}Socket;


/* Socket Base */
int     platformInit();
void    sockaddrInit(Socket* server, uint32_t address, uint16_t port);
int     socketInit(Socket* server, int IPPROTO);
void    socketClose(Socket*);
void    setTimeOut(Socket*, uint32_t send_timeout, uint32_t recv_timeout);


/* New Thread Base */
thread_t    threadCreate(void* (*thread_handler)(void*), void* args);
void        threadExit(size_t sleep_time_ms);
void        threadDetach(thread_t);
int         threadJoin(thread_t thread,void** retVal);

/* Thread Lock */

typedef pthread_rwlock_t rwlock_t;

int lockInit(rwlock_t*);
int readLock(rwlock_t*);
int unlock(rwlock_t*);
int writeLock(rwlock_t*);
int lockDestroy(rwlock_t*);


#endif