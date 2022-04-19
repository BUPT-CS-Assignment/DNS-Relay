#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>

#define LOCAL_DNS_ADDR  "10.21.234.164" 
#define BUFFER_SIZE 1024

typedef struct{
    SOCKET socket;
    struct sockaddr_in sock_addr;
    struct sockaddr_in local_dns;
}Server;

typedef struct{
    char *QNAME;
    uint16_t QTYPE;
}Quest;

typedef struct{
    char *NAME;
}Answer;

/* Packet form */
typedef struct{
    /* Origin Quest */
    char *req_buf;
    int buf_len;
    /* Header Section */
    uint16_t ID, FLAGS, QDCOUNT;
    /* Question Section */
    Quest *QUESTS;
    /* Answer Section */
    Answer *ANS;

}Packet;

/* ServerBase */
void Start(Server *);
int ServerInit(Server *);
void RecvHandle(char *, int,struct sockaddr);
void PacketFree(Packet *);

/* Packet Handle */
Packet *RequestParse(char *buf, int len);
char *ResponseFormat(Packet, int *, char **url);

/* Packet Parse */
char *UrlParse(char *);
uint32_t UrlFormat(char *);

/* Packet Check */
void PacketCheck(Packet *packet);
void BuffCheck(char*,int);
#endif
