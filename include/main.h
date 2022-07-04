#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32

#include <winsock2.h>

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

#endif

extern int __DEBUG__;

#endif