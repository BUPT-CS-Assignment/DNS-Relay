#ifndef __SERVER_H__
#define __SERVER_H__

#include "main.h"
#include "protocol.h"
#include "cache.h"

/*----------------------------------- Definitions -----------------------------------*/

static char _local_dns_addr[64] = "114.114.114.114";

/*-------------------------------- Global Variables ---------------------------------*/


extern Socket _dns_server;      //local dns server
extern int LRU_CACHE_LENGTH;
extern LRU_cache* _url_cache;

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


/*----------------------------------- Functions ------------------------------------*/


/* Socket Base */
int     socketInit(Socket* server, uint32_t address, uint16_t port);
void    socketClose(Socket*);
void    setTimeOut(Socket*, uint32_t send_timeout, uint32_t recv_timeout);


/* Server Base */
void    start(Socket*);
void*   connectHandle(void* param);


/* Address Query */
int     urlStore(Packet*);
int     urlQuery(Packet*);
int     qnameSearch(char *qname,uint16_t qtype,DNS_entry** result);


/* Packet Handle */
Packet* packetParse(uint8_t* buf, int len);
char*   responseFormat(int* len, Packet*);
void    packetFree(Packet*);


/* Url Resolve */
int     urlFormat(char* url, void* dest, int mode, char* name, uint16_t pointer, uint16_t addition);
int     urlParse(void* src, char* dest, void* addtion,int mode, uint16_t len, uint8_t* buf);


/* Packet Check */
void    packetCheck(Packet*);
void    bufferCheck(char* buf, int len);


#endif
