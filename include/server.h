#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>
#include <protocol.h>
#include <cache.h>

/*----------------------------------- Definitions -----------------------------------*/

static char _local_dns_addr[64] = "114.114.114.114";

/*-------------------------------- Global Variables ---------------------------------*/

/* Static Records */
static char* RECORDS[][2] = {
    {"bing.com","204.79.197.200"},
    {"bing.com","13.107.21.200"},
    {"bing.com","2620:1ec:c11::300"},
    {"bupt.edu.cn","10.3.9.161"},
    {"noui.cloud","101.43.201.20"},
    {"unique.com","220.177.198.124"}
};
static int R_NUM = 6;   //Records Num

extern Socket _dns_server;  //local dns server
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
