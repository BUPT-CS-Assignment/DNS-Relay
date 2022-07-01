#include <server.h>
#include <console.h>

/**
 *      DNS PACKET STRUCTURE
 *
 *      +-----------------------------------------------+
 *      |               HEADER   SECTION                |
 *      +-----------------------------------------------+
 *      /              QUESTION  SECTION                /
 *      /                                               /
 *      +-----------------------------------------------+
 *      /               ANSWER   SECTION                /
 *      /                                               /
 *      +-----------------------------------------------+
 *      /              AUTHORITY SECTION                /
 *      /                                               /
 *      +-----------------------------------------------+
 *      /             ADDITIONAL SECTION                /
 *      /                                               /
 *      +-----------------------------------------------+
 *
 */


 /**
  * @brief Parse Packet
  *
  * @param char* buf
  * @param int len
  * @return Packet* packet
  */
Packet *packetParse(char *buf, int len){

    Packet *dest = (Packet *)malloc(sizeof(Packet));
    memset(dest, 0, sizeof(Packet));
    dest->ANS = NULL;

    /* set req_buf */
    dest->req_buf = buf;
    dest->buf_len = len;


    /* Parse Header Section
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+-----------+--+--+--+--+--------+-----------+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+-----------+--+--+--+--+--------+-----------+
    |                    QDCOUNT                    |
    +-----------------------------------------------+
    |                    ANCOUNT                    |
    +-----------------------------------------------+
    |                    NSCOUNT                    |
    +-----------------------------------------------+
    |                    ARCOUNT                    |
    +-----------------------------------------------+
    */
    memcpy(&dest->ID, buf, sizeof(uint16_t));
    dest->FLAGS = ntohs(*((uint16_t *)(buf + 2)));
    dest->QDCOUNT = ntohs(*((uint16_t *)(buf + 4)));
    dest->ANCOUNT = ntohs(*(uint16_t *)(buf + 6));
    char *buf_pos = buf + 12;


    /* Parse Question Section
     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    /                    QNAME                      /
    /                                               /
    +-----------------------------------------------+
    |                    QTYPE                      |
    +-----------------------------------------------+
    |                    QCLASS                     |
    +-----------------------------------------------+
    */
    dest->QUESTS = (Quest *)malloc(sizeof(Quest) * dest->QDCOUNT);
    for(int i = 0; i < dest->QDCOUNT; i++){
        /* read till '\0' */
        int q_len = strlen(buf_pos);

        /* QNAME　parse */
        dest->QUESTS[i].QNAME = (char *)malloc(260);
        urlParse(buf_pos, dest->QUESTS[i].QNAME, TYPE_QNAME);

        /* QTYPE parse */
        buf_pos += (q_len + 1);
        dest->QUESTS[i].QTYPE = ntohs(*((uint16_t *)buf_pos));
        dest->QUESTS[i].QCLASS = ntohs(*(uint16_t *)(buf_pos + 2));
        buf_pos += 4;
    }

    if(dest->ANCOUNT == 0)    return dest;


    /*Parse Answer Section
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     NAME                      |
    +-----------------------------------------------+
    |                     TYPE                      |
    +-----------------------------------------------+
    |                     CLASS                     |
    +-----------------------------------------------+
    |                      TTL                      |
    |                                               |
    +-----------------------------------------------+
    |                   RDLENGTH                    |
    +-----------------------------------------------+
    /                    RDATA                      /
    /                                               /
    +-----------------------------------------------+
    */
    dest->ANS = (Answer *)malloc(sizeof(Answer) * dest->ANCOUNT);
    for(int i = 0; i < dest->ANCOUNT; i++){
        /* Set Basic Info */
        dest->ANS[i].NAME = GET_QNAME_PTR(ntohs(*(uint16_t *)buf_pos));
        dest->ANS[i].TYPE = ntohs(*(uint16_t *)(buf_pos + 2));
        dest->ANS[i].CLASS = ntohs(*(uint16_t *)(buf_pos + 4));
        dest->ANS[i].TTL = ntohl(*(uint32_t *)(buf_pos + 6));

        /* Set Resource Info*/
        dest->ANS[i].RDLEN = ntohs(*(uint16_t *)(buf_pos + 10));
        dest->ANS[i].RDATA = (char *)malloc(17);
        urlParse(buf_pos + 12, dest->ANS[i].RDATA, TYPE_A);
        buf_pos += (12 + dest->ANS[i].RDLEN);
    }
    return dest;
}


/**
 * @brief Construct Response Packet Buffer
 *
 * @param int* len
 * @param Packet* src
 * @param char** url
 * @return char* buff
 */
char *responseFormat(int *len, Packet *src){

    /* Set Flags
    +--+-----------+--+--+--+--+--------+-----------+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+-----------+--+--+--+--+--------+-----------+
    */
    uint16_t flag = src->FLAGS;
    SET_QR(flag);
    SET_RD(flag);

    /* Answer Section */
    if(src->ANCOUNT == 0){
        SET_AA(flag);
        SET_RCODE(flag, RCODE_NAME_ERROR);
    }
    flag = ntohs(flag);

    /* -----------------------------IMPROVE-REQUIRED-0-----------------------------------*/
    /* Set Name-Pointer
    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | 1| 1|             QNAME Pointer               |
    +--+-----------+--+--+--+--+--------+-----------+
         ^ Pointer Recognize
    */
    uint16_t names[src->ANCOUNT];
    names[0] = 0xc00c;  //Pos = 1100000000001100 
    for(int i = 1; i < src->ANCOUNT; i++){
        names[i] = names[i - 1]  /* + strlen(src->QUESTS[i].QNAME) + 5 */ ;
    }
    for(int i = 0; i < src->ANCOUNT; i++){
        names[i] = htons(names[i]);
    }

    /* -----------------------------IMPROVE-REQUIRED-0------------------------------------*/

    /* ResData Resolve */
    uint32_t resData[src->ANCOUNT];
    for(int i = 0; i < src->ANCOUNT; i++){
        resData[i] = 0;
        urlFormat(src->ANS[i].RDATA, &resData[i], src->ANS[i].TYPE);
        resData[i] = htonl(resData[i]);
    }

    /* Memory Allocated */
    /* -----------------------------IMPROVE-REQUIRED-1------------------------------------*/

    *len = src->buf_len + src->ANCOUNT * 16;
    char *dest = (char *)malloc(*len + 1);

    /* -----------------------------IMPROVE-REQUIRED-1------------------------------------*/

    /* Set Basic Dest Info */
    memcpy(dest, src->req_buf, src->buf_len);
    memcpy(dest + 2, &flag, sizeof(uint16_t));


    /* Set Answer Section
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     NAME                      |
    +-----------------------------------------------+
    |                     TYPE                      |
    +-----------------------------------------------+
    |                     CLASS                     |
    +-----------------------------------------------+
    |                      TTL                      |
    |                                               |
    +-----------------------------------------------+
    |                   RDLENGTH                    |
    +-----------------------------------------------+
    /                    RDATA                      /
    /                                               /
    +-----------------------------------------------+
    */
    uint16_t ancount = htons(src->ANCOUNT);
    memcpy(dest + 6, &ancount, sizeof(uint16_t));    /* Translate ANCOUNT */

    /* set data section */
    for(int i = 0; i < src->ANCOUNT; i++){
        char *dataPos = dest + (src->buf_len + (i * 16));
        Answer *pANS = &(src->ANS[i]);
        uint16_t type = htons(pANS->TYPE);
        uint16_t class = htons(pANS->CLASS);
        uint32_t ttl = htonl(pANS->TTL);
        memcpy(dataPos, &names[i], sizeof(uint16_t));         /* Set NAME poiner 16 Bits */
        memcpy((dataPos + 2), &type, sizeof(uint16_t));      /* Set TYPE 16 Bits */
        memcpy((dataPos + 4), &class, sizeof(uint16_t));      /* Set CLASS 16 Bits */
        memcpy((dataPos + 6), &ttl, sizeof(uint32_t));       /* Set TTL 32 Bits */
        memcpy((dataPos + 12), &resData[i], pANS->RDLEN);         /* Set RDATA */
        int dataLen = htons(pANS->RDLEN);
        memcpy((dataPos + 10), &dataLen, sizeof(uint16_t));   /* Set RDATA Length 16 Bits */
    }
    return dest;

}


/**
 * @brief Check Packet Struct Info
 *
 * @param Packet* src
 */
void packetCheck(Packet *src){
    printf("> Packet Check\n");
    if(src == NULL){
        printf("<null-ptr>\n");
    }
    /* Origin Buffer Check */
    bufferCheck(src->req_buf, src->buf_len);

    /* Check Header */
    printf(" - ID= %d\n", src->ID);

    /* Check FLAGS */
    printf("   QR= %d  RD= %d  RA= %d  RCODE= %d\n", GET_QR(src->FLAGS), GET_RD(src->FLAGS),
        GET_RA(src->FLAGS), GET_RCODE(src->FLAGS));

    /* Check COUNTS */
    printf("   QDCOUNT= %d  ANCOUNT= %d\n", src->QDCOUNT, src->ANCOUNT);

    /* Check Question Section */
    for(int i = 0; i < src->QDCOUNT; i++){
        printf(" - QNAME(%d)= '%s'\n", i, src->QUESTS[i].QNAME);
        printf("   QTYPE(%d)= %d  QCLASS(%d)= %d\n", i, src->QUESTS[i].QTYPE, i, src->QUESTS[i].QCLASS);
    }

    /* Check Answer Section */
    for(int i = 0; i < src->ANCOUNT; i++){
        printf(" - NAMEPTR(%d)= %d\n", i, src->ANS[i].NAME);
        printf("   TYPE(%d)= %d  CLASS(%d)= %d  TTL(%d)= %u\n", i, src->ANS[i].TYPE,
            i, src->ANS[i].CLASS, i, src->ANS[i].TTL);
        printf("   RDLEN(%d)= %d  RDATA(%d)= '%s'\n", i, src->ANS[i].RDLEN, i, src->ANS[i].RDATA);
    }
    printf("> Check End\n");
}


/**
 * @brief Free Packet Memory
 *
 * @param Packet* src
 */
void packetFree(Packet *src){
    /* Free Question Section */
    if(src->QUESTS != NULL){
        for(int i = 0; i < src->QDCOUNT; i++){
            free(src->QUESTS[i].QNAME);
        }
        free(src->QUESTS);
    }

    /* Free Answer Section */
    if(src->ANS != NULL){
        for(int i = 0; i < src->ANCOUNT; i++){
            free(src->ANS[i].RDATA);
        }
        free(src->ANS);
    }
    src->req_buf = NULL;

    /* Free Whole Struct */
    free(src);
}


/**
 * @brief Check Packet Buffer Info
 *
 * @param char* buf
 * @param int len
 */
void bufferCheck(char *buf, int len){
    if(buf == NULL || len <= 0){
        printf("<none buff>\n");
        return;
    }
    printf(" - Packet Length= %d\n   [Packet] ", len);
    for(int i = 0; i < len; i++){
        /* Format Output */
        printf("%02X ", (unsigned char)buf[i]);
        if(i == len - 1){
            printf("\n");
        }
        else if(i > 0 && (i + 1) % 16 == 0){
            printf("\n            ");
        }
    }
}