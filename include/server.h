#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>

/************************************ Definitions ************************************/


#define DNS_RELAY_ADDR          INADDR_ANY
#define LOCAL_DNS_ADDR          inet_addr("114.114.114.114")

/* MAX CONNECT NUM */
#define MAX_CONNECT             128

/* TYPE CODE*/
#define TYPE_QNAME              -1
#define TYPE_A                  1
#define TYPE_NA                 2
#define TYPE_CNAME              5
#define TYPE_MX                 15
#define TYPE_TXT                16
#define TYPE_AAAA               28

/* TYPE SIZE */
#define SIZE_TYPE_A             4
#define SIZE_TYPE_AAAA          16

/* RCODE */
#define RCODE_NO_ERROR          0x0
#define RCODE_FORMAT_ERROR      0x1
#define RCODE_SERVER_FAILURE    0x2
#define RCODE_NAME_ERROR        0x3
#define RCODE_NOT_IMPLEMENTED   0x4
#define RCODE_REFUSED           0x5

/* ID OPERATION */
#define SET_ID(BUF,ID)          memcpy(BUF,ID,sizeof(uint16_t));
#define GET_ID(BUF)             ({(*(uint16_t *)BUF);})

/* Set FLAGS */
#define SET_QR(FLAG)            FLAG |= 0x8000
#define SET_AA(FLAG)            FLAG |= 0x400
#define SET_RD(FLAG)            FLAG |= 0x100
#define SET_RA(FLAG)            FLAG |= 0x80
#define SET_RCODE(FLAG,CODE)    FLAG = (FLAG & 0xfff0) | CODE

/* Reset FLAGS */
#define RESET_QR(FLAG)          FLAG &= !0x8000
#define RESET_AA(FLAG)          FLAG &= !0x400
#define RESET_RD(FLAG)          FLAG &= !0x100
#define RESET_RA(FLAG)          FLAG &= !0x80

/* Get FLAGS */
#define GET_QR(FLAG)            ({((FLAG & 0x8000) >> 15);})
#define GET_RD(FLAG)            ({((FLAG & 0x100) >> 8);})
#define GET_RA(FLAG)            ({((FLAG & 0x80) >> 7);})
#define GET_RCODE(FLAG)         ({(FLAG & 0x000f);})
#define GET_QNAME_PTR(NAME)     ({NAME & 0x3fff;})

/**
 * @brief DNS Question Section Struct
 * 
 */
typedef struct Quest
{
    char* QNAME;
    uint16_t  QTYPE;
    uint16_t  QCLASS;

}Quest;


/**
 * @brief DNS Answer Section Struct 
 * 
 */
typedef struct Answer
{
    uint16_t  NAME;
    uint16_t  TYPE;
    uint16_t  CLASS;
    uint16_t  RDLEN;
    uint32_t  TTL;
    char* RDATA;

}Answer;


/**
 * @brief Packet Form Struct
 * 
 */
typedef struct Packet
{
    /* Origin Infomation */
    char* req_buf;      //origin request buffer pointer       
    int       buf_len;      //origin request buffer length

    /* Header Section */
    uint16_t  ID;
    uint16_t  FLAGS;
    uint16_t  QDCOUNT;      //number of questions
    uint16_t  ANCOUNT;      //number of answers

    /* Question Section */
    Quest* QUESTS;

    /* Answer Section */
    Answer* ANS;

}Packet;


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
size_t  getBufferSize(uint16_t type);
size_t  getTypeSize(uint16_t type);
char*   responseFormat(int* len, Packet*);
void    packetFree(Packet*);


/* Url Resolve */
int     urlFormat(char* src, void* dest, int mode);
int     urlParse(void* src, char* dest, int mode);


/* Packet Check */
void    packetCheck(Packet*);
void    bufferCheck(char* buf, int len);


#endif
