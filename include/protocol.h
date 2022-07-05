#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* TYPE CODE*/
#define TYPE_QNAME              0
#define TYPE_A                  1
#define TYPE_NS                 2
#define TYPE_CNAME              5
#define TYPE_MX                 15
#define TYPE_TXT                16
#define TYPE_AAAA               28

/* GET TYPE SIZE ON NET PARSE */
static size_t TYPE_SIZE(uint16_t TYPE){
    switch(TYPE){
        case TYPE_A:            return 4;
        case TYPE_AAAA:         return 16;
        case TYPE_NS:           
        case TYPE_MX:
        case TYPE_CNAME:        return 255;
        default:                return 0;
    }
}

/* TYPE TYPE BUFFER SIZE FOR PARSE */
static size_t TYPE_BUF_SIZE(uint16_t TYPE){
    switch(TYPE){
        case TYPE_A:            return 16;
        case TYPE_AAAA:         return 40;
        case TYPE_NS:           
        case TYPE_MX:
        case TYPE_CNAME:        return 255;
        default:                return 0;
    }
}

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
    char*       QNAME;
    uint16_t    QTYPE;
    uint16_t    QCLASS;

}Quest;


/**
 * @brief DNS Answer Section Struct 
 * 
 */
typedef struct Answer
{
    uint8_t     QPOS;
    uint16_t    NAME;
    uint16_t    TYPE;
    uint16_t    CLASS;
    uint16_t    RDLEN;
    uint32_t    TTL;
    char*       RDATA;

}Answer;


/**
 * @brief Packet Form Struct
 * 
 */
typedef struct Packet
{
    /* Origin Infomation */
    char*       req_buf;      //origin request buffer pointer       
    int         buf_len;      //origin request buffer length

    /* Header Section */
    uint16_t    ID;
    uint16_t    FLAGS;
    uint16_t    QDCOUNT;      //number of questions
    uint16_t    ANCOUNT;      //number of answers

    /* Question Section */
    Quest*      QUESTS;

    /* Answer Section */
    Answer*     ANS;

}Packet;



#endif