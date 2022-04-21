#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>

#define LOCAL_DNS_ADDR  "10.21.234.164" 
/* MAX BUFFER SIZE */
#define BUFFER_SIZE 1024
/* TYPE CODE*/
#define TYPE_QNAME -1
#define TYPE_A 1
#define TYPE_NA 2
#define TYPE_CNAME 5
#define TYPE_MX 15
#define TYPE_TXT 16
#define TYPE_AAAA 28
/* TYPE SIZE */
#define SIZE_TYPE_A 4
#define SIZE_TYPE_AAAA 16
/* RCODE */
#define RCODE_NO_ERROR 0x0
#define RCODE_FORMAT_ERROR 0x1
#define RCODE_SERVER_FAILURE 0x2
#define RCODE_NAME_ERROR 0x3
#define RCODE_NOT_IMPLEMENTED 0x4
#define RCODE_REFUSED 0x5
/* SET FLAG BITS */
#define SET_QR(FLAG) FLAG |= 0x8000
#define RESET_QR(FLAG) FLAG &= !0x8000
#define SET_AA(FLAG) FLAG |= 0x400
#define RESET_AA(FLAG) FLAG &= !0x400
#define SET_RD(FLAG) FLAG |= 0x100
#define RESET_RD(FLAG) FLAG &= !0x100
#define SET_RA(FLAG) FLAG |= 0x80
#define RESET_RA(FLAG) FLAG &= !0x80
/* SET RCODE */
#define SET_RCODE(FLAG,CODE) FLAG = (FLAG & 0xfff0) | CODE


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
    uint16_t NAME;
    char *RDATA;
}Answer;

/* Packet form */
typedef struct{
    /* Origin Quest */
    char *req_buf;
    int buf_len;
    /* Header Section */
    uint16_t ID, FLAGS, QDCOUNT, ANCOUNT;
    /* Question Section */
    Quest *QUESTS;
    /* Answer Section */
    Answer *ANS;

}Packet;

/* ServerBase */
void Start(Server *);
int ServerInit(Server *);
void RecvHandle(char *, int, struct sockaddr);
void PacketFree(Packet *);

/* Packet Handle */
Packet* PacketParse(char *buf, int len);
char* ResponseFormat(int*,Packet*,char **urls);
/* Url Handle */

int UrlFormat(char *src,void* dest,int mode);
int UrlParse(void* src,char* dest,int mode);

/* Packet Check */
void PacketCheck(Packet *packet);
void BuffCheck(char *, int);

#endif
