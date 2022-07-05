#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

#define ERROR_CODE GetLastError()

#else

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef int SOCKET;

#define SOCKET_ERROR -1
#define ERROR_CODE errno

#endif

/* Debug setting */
#define DEBUG_L0    0
#define DEBUG_L1    1
#define DEBUG_L2    2
extern int          __DEBUG__;


/* inet change */
int inetParse(int af,void* url_src,char* dest);
int inetFormat(int af,char* url_src,void* dest);



/* Basic Socket Struct Contents FD & ADDR */
typedef struct Socket
{
    SOCKET _fd;
    struct sockaddr_in _addr;

}Socket;

/* MAX BUFFER SIZE */
#define BUFFER_SIZE             1024

/* New Thread Args */
typedef struct thread_args
{
    char buf[BUFFER_SIZE];
    Socket* server;
    Socket connect;
    int buf_len;

}thread_args;

/* New Thread Create */
void    threadCreate(void*(*thread_handler)(void*),thread_args*);
void    threadExit();


#endif