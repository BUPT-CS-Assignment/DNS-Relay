#include "server.h"
#include "console.h"

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
  * @brief parse Packet
  *
  * @param buf package buffer pointer
  * @param len package buffer length
  * @return Packet* Packet pointer
  */
Packet* packetParse(uint8_t* buf, int len)
{

    Packet* dest = (Packet*)malloc(sizeof(Packet));
    memset(dest, 0, sizeof(Packet));
    dest->ANS = NULL;

    /* set req_buf */
    dest->req_buf = buf;
    dest->buf_len = len;


    /* --------------------------------- Header Section ---------------------------------*/
    /*
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
    //memcpy(&dest->ID, buf, sizeof(uint16_t));
    dest->ID = GET_ID(buf);
    dest->FLAGS = ntohs(*((uint16_t*)(buf + 2)));
    dest->QDCOUNT = ntohs(*((uint16_t*)(buf + 4)));
    dest->ANCOUNT = ntohs(*(uint16_t*)(buf + 6));
    char* buf_pos = buf + 12;


    /* --------------------------------- Question Section ---------------------------------*/
    /*
     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    /                    QNAME                      /
    /                                               /
    +-----------------------------------------------+
    |                    QTYPE                      |
    +-----------------------------------------------+
    |                    QCLASS                     |
    +-----------------------------------------------+
    */
    dest->QUESTS = (Quest*)malloc(sizeof(Quest) * dest->QDCOUNT);
    for(int i = 0; i < dest->QDCOUNT; i++)
    {
        /* read till '\0' */
        int q_len = strlen(buf_pos);

        /* QNAME　parse */
        dest->QUESTS[i].QNAME = (char*)malloc(64 + 1);
        urlParse(buf_pos, dest->QUESTS[i].QNAME, NULL, TYPE_QNAME, 0, NULL);

        /* QTYPE parse */
        buf_pos += (q_len + 1);
        dest->QUESTS[i].QTYPE = ntohs(*((uint16_t*)buf_pos));
        dest->QUESTS[i].QCLASS = ntohs(*(uint16_t*)(buf_pos + 2));
        buf_pos += 4;
    }

    if(dest->ANCOUNT == 0)    return dest;


    /* --------------------------------- Answer Section ---------------------------------*/
    /*
     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
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
    dest->ANS = (Answer*)malloc(sizeof(Answer) * dest->ANCOUNT);
    int name_qpos = 0;
    for(int i = 0; i < dest->ANCOUNT; i++)
    {
        /* Set Basic Info */
        dest->ANS[i].NAME = *(uint16_t*)buf_pos;
        dest->ANS[i].QPOS = 0;
        //(i == 0 ? 0 : (dest->ANS[i].NAME == dest->ANS[i - 1].NAME ? name_qpos : ++name_qpos));
        dest->ANS[i].TYPE = ntohs(*(uint16_t*)(buf_pos + 2));
        dest->ANS[i].CLASS = ntohs(*(uint16_t*)(buf_pos + 4));
        dest->ANS[i].TTL = ntohl(*(uint32_t*)(buf_pos + 6));

        /* Set Resource Info*/
        dest->ANS[i].RDLEN = ntohs(*(uint16_t*)(buf_pos + 10));

        /* Parse Url */
        dest->ANS[i].RDATA = (uint8_t*)malloc(TYPE_BUF_SIZE(dest->ANS[i].TYPE));
        urlParse(buf_pos + 12, (char*)dest->ANS[i].RDATA, &dest->ANS[i].ADDITION,
            dest->ANS[i].TYPE, dest->ANS[i].RDLEN, buf);
        buf_pos += (12 + dest->ANS[i].RDLEN);
    }
    return dest;
}




/**
 * @brief generate response package buffer
 *
 * @param len buffer length pointer
 * @param src Packet pointer
 * @return char* response package buffer pointer
 */
char* responseFormat(int* len, Packet* src)
{

    /* --------------------------------- Flag Section ---------------------------------*/
    /* Set Flags
     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    +--+-----------+--+--+--+--+--------+-----------+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+-----------+--+--+--+--+--------+-----------+
    */
    uint16_t flag = src->FLAGS;
    SET_QR(flag);   SET_RD(flag);

    /* Answer Section */
    if(src->ANCOUNT == 0 || src->ANCOUNT == UINT16_MAX)
    {
        SET_AA(flag);
        SET_RCODE(flag, RCODE_NAME_ERROR);
    }
    flag = ntohs(flag);

    /* --------------------------------- QName Pointer ---------------------------------*/
    /* Set Name-Pointer
     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | 1| 1|             QNAME Pointer               |
    +--+-----------+--+--+--+--+--------+-----------+
         ^ Pointer Recognize
    */
    uint16_t names[src->ANCOUNT];
    //uint16_t p_offset;          //quset section offset      
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        names[i] = 0xc00c;      //pos = 1100000000001100 
        for(int j = 0; j < src->ANS[i].QPOS; j++)
        {
            names[i] += strlen(src->QUESTS[j].QNAME) + 1 + 4; //QNAME + QTYPE(2) + QCLASS(2)
        }
        //names[i] = htons(p_offset);
    }

    /* --------------------------------- Answer Section ---------------------------------*/

    /* ResData Resolve */
    *len = src->buf_len;
    uint8_t resData[255];
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        /* format transform & get length  */
        char* origin_name = src->QUESTS[src->ANS[i].QPOS].QNAME;
        src->ANS[i].RDLEN = (uint16_t)urlFormat(src->ANS[i].RDATA, &resData, src->ANS[i].TYPE,
            origin_name, names[i], src->ANS[i].ADDITION);

        memcpy(src->ANS[i].RDATA,&resData,src->ANS[i].RDLEN);
        *len += src->ANS[i].RDLEN + 12;
    }

    /* Memory Allocated */
    char* dest = (char*)malloc(*len + 1);


    /* Set Basic Dest Info */
    memcpy(dest, src->req_buf, src->buf_len);
    memcpy(dest + 2, &flag, sizeof(uint16_t));


    /* Set Answer Section
     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                 NAME (POINTER)                |
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
    memcpy(dest + 6, &ancount, sizeof(uint16_t));    // Transform ANCOUNT

    /* set data section */
    char* dataPos = dest + src->buf_len;
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        Answer* pANS = &(src->ANS[i]);

        /* transform origin data */
        uint16_t name_ptr = htons(names[i]);
        uint16_t type = htons(pANS->TYPE);
        uint16_t class = htons(pANS->CLASS);
        uint16_t dataLen = htons(pANS->RDLEN);
        uint32_t ttl = htonl(pANS->TTL);

        /* copy data to data buffer */
        memcpy((dataPos + 0), &name_ptr, sizeof(uint16_t));     // Set NAME poiner  16 Bits
        memcpy((dataPos + 2), &type, sizeof(uint16_t));         // Set TYPE         16 Bits
        memcpy((dataPos + 4), &class, sizeof(uint16_t));        // Set CLASS        16 Bits
        memcpy((dataPos + 6), &ttl, sizeof(uint32_t));          // Set TTL          32 Bits
        memcpy((dataPos + 12), pANS->RDATA, pANS->RDLEN);       // Set RDATA
        memcpy((dataPos + 10), &dataLen, sizeof(uint16_t));     // Set RDATA Length 16 Bits
        
        dataPos += pANS->RDLEN + 12;                            // Set dest-pointer offset
    }
    return dest;

}




/**
 * @brief check Packet infomation
 *
 * @param src Packet pointer
 */
void packetCheck(Packet* src)
{
    if(__DEBUG__ == DEBUG_L0 || src == NULL)   return;
    printf("> Packet Check\n");

    /* Origin Buffer Check */
    bufferCheck(src->req_buf, src->buf_len);

    /* Check Header */
    printf(" - ID= %d\n", src->ID);

    /* Check FLAGS */
    printf("   "BOLDBLUE"QR= %d"RESET"  RD= %d  RA= %d  RCODE= %d\n", GET_QR(src->FLAGS), GET_RD(src->FLAGS),
        GET_RA(src->FLAGS), GET_RCODE(src->FLAGS));

    /* Check COUNTS */
    printf("   QDCOUNT= %d  ", src->QDCOUNT);
    if(GET_QR(src->FLAGS) == 1) printf(BOLDMAGENTA);
    printf("ANCOUNT= %d\n"RESET, src->ANCOUNT);

    /* Check Question Section */
    for(int i = 0; i < src->QDCOUNT; i++)
    {
        printf(" - QNAME(%d)= '%s'\n", i, src->QUESTS[i].QNAME);
        printf("   "BOLDMAGENTA"QTYPE(%d)= %d"RESET"  QCLASS(%d)= %d\n",
            i, src->QUESTS[i].QTYPE, i, src->QUESTS[i].QCLASS);
    }

    /* Check Answer Section */
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        printf(" - NAMEPTR(%d)= %04x\n", i, htons(src->ANS[i].NAME));
        printf("   TYPE(%d)= %d  CLASS(%d)= %d  TTL(%d)= %d\n", i, src->ANS[i].TYPE,
            i, src->ANS[i].CLASS, i, src->ANS[i].TTL);
        printf("   RDLEN(%d)= %d  RDATA(%d)= '%s'", i, src->ANS[i].RDLEN, i, src->ANS[i].RDATA);
        if(src->ANS[i].TYPE == TYPE_MX)
        {
            printf("  PREF(%d)= %d", i, src->ANS[i].ADDITION);
        }
        printf("\n");
    }
    printf("> Check End\n");
}



/**
 * @brief free Packet memory
 *
 * @param src Packet pointer
 */
void packetFree(Packet* src)
{
    /* Free Question Section */
    if(src->QUESTS != NULL)
    {
        for(int i = 0; i < src->QDCOUNT; i++)
        {
            free(src->QUESTS[i].QNAME);
        }
        free(src->QUESTS);
    }

    /* Free Answer Section */
    if(src->ANS != NULL)
    {
        for(int i = 0; i < src->ANCOUNT; i++)
        {
            free(src->ANS[i].RDATA);
        }
        free(src->ANS);
    }
    src->req_buf = NULL;

    /* Free Whole Struct */
    free(src);
}



/**
 * @brief check package buffer infomation
 *
 * @param buf package buffer pointer
 * @param len package buffer length
 */
void bufferCheck(char* buf, int len)
{
    if(buf == NULL || len <= 0)
    {
        printf("<null-pointer>\n");
        return;
    }
    printf(" - Packet Length= %d\n", len);

    if(__DEBUG__ < DEBUG_L2)    return;

    printf("   [Origin] ");
    for(int i = 0; i < len; i++)
    {
        /* Format Output */
        printf("%02X ", (unsigned char)buf[i]);
        if(i == len - 1)
        {
            printf("\n");
        }
        else if(i > 0 && (i + 1) % 16 == 0)
        {
            printf("\n            ");
        }
    }
}


