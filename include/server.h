#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>
#include <protocol.h>

/************************************ Definitions ************************************/


#define DNS_RELAY_ADDR          INADDR_ANY
#define LOCAL_DNS_ADDR          inet_addr("114.114.114.114")


/********************************** Global Variables *********************************/

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


/************************************ Functions **************************************/


/* Socket Base */
int     socketInit(Socket* server, uint32_t address, uint16_t port);
void    socketClose(Socket*);
void    setTimeOut(Socket*, uint32_t send_timeout, uint32_t recv_timeout);


/* Server Base */
void    start(Socket*);
void*   connectHandle(void* param);


/* Address Query */
int     urlQuery(Packet*, char*** records, int num);
int     qnameSearch(char *src,int type,int *res, char ***records, int urls_num);


/* Packet Handle */
Packet* packetParse(char* buf, int len);
char*   responseFormat(int* len, Packet*);
void    packetFree(Packet*);


/* Url Resolve */
int     urlFormat(char* src, void* dest, int mode);
int     urlParse(void* src, char* dest, int mode);


/* Packet Check */
void    packetCheck(Packet*);
void    bufferCheck(char* buf, int len);


#endif
