#include <server.h>
#include <console.h>

Packet *PacketParse(char *buf, int len){

    Packet *dest = (Packet *)malloc(sizeof(Packet));
    memset(dest, 0, sizeof(Packet));
    dest->ANS = NULL;

    /* set req_buf */
    dest->req_buf = buf;
    dest->buf_len = len;

    /* set Header */
    memcpy(&dest->ID, buf, sizeof(uint16_t));
    dest->FLAGS = ntohs(*((uint16_t *)(buf + 2)));
    dest->QDCOUNT = ntohs(*((uint16_t *)(buf + 4)));
    dest->ANCOUNT = ntohs(*(uint16_t *)(buf + 6));
    char *buf_pos = buf + 12;

    /* Get Questions */
    dest->QUESTS = (Quest *)malloc(sizeof(Quest) * dest->QDCOUNT);
    for(int i = 0; i < dest->QDCOUNT; i++){

        /* read till '\0' */
        int q_len = strlen(buf_pos);

        /* QNAME　parse */
        dest->QUESTS[i].QNAME = (char *)malloc(260);
        UrlParse(buf_pos, dest->QUESTS[i].QNAME, TYPE_QNAME);

        /* QTYPE parse */
        buf_pos += (q_len + 1);
        dest->QUESTS[i].QTYPE = ntohs(*((uint16_t *)buf_pos));
        dest->QUESTS[i].QCLASS = ntohs(*(uint16_t *)(buf_pos + 2));
        buf_pos += 4;
    }

    if(dest->ANCOUNT == 0)    return dest;

    /* Get Answers */
    dest->ANS = (Answer *)malloc(sizeof(Answer) * dest->ANCOUNT);
    for(int i = 0; i < dest->ANCOUNT; i++){
        dest->ANS[i].NAME = GET_QNAME_PTR(ntohs(*(uint16_t *)buf_pos));
        dest->ANS[i].TYPE = ntohs(*(uint16_t *)(buf_pos + 2));
        dest->ANS[i].CLASS = ntohs(*(uint16_t *)(buf_pos + 4));
        dest->ANS[i].TTL = ntohl(*(uint32_t *)(buf_pos + 6));
        dest->ANS[i].RDLEN = ntohs(*(uint16_t *)(buf_pos + 10));
        dest->ANS[i].RDATA = (char *)malloc(dest->ANS[i].RDLEN + 1);
        UrlParse(buf_pos+12, dest->ANS[i].RDATA, TYPE_A);
        buf_pos += dest->ANS[i].RDLEN;
    }
    return dest;
}




char *ResponseFormat(int *len, Packet *src, char **url){

    /* Set Flags */
    uint16_t flag = src->FLAGS;
    SET_QR(flag);
    SET_RD(flag);
    if(src->QUESTS[0].QTYPE != TYPE_A){
        SET_RCODE(flag, RCODE_NOT_IMPLEMENTED);
        src->ANCOUNT = 0;
    }
    flag = ntohs(flag);

    /* Set Answer Section */
    uint16_t names[src->QDCOUNT];
    names[0] = 0xc00c;  //Pos = 1100000000001100 
    for(int i = 1; i < src->QDCOUNT; i++){
        names[i] = names[i - 1] + strlen(src->QUESTS[i].QNAME) + 5;
    }
    for(int i = 0; i < src->QDCOUNT; i++){
        names[i] = htons(names[i]);
    }

    if(strcmp(src->QUESTS[0].QNAME, "aaa.com") == 0){
        src->ANCOUNT = 1;
    }
    /* ResData Resolve */
    uint32_t resData[src->ANCOUNT];
    for(int i = 0; i < src->ANCOUNT; i++){
        resData[i] = 0;
        UrlFormat(url[i], &resData[i], TYPE_A);
        resData[i] = htonl(resData[i]);
    }
    /* Memory Allocated */
    /* ------------------------IMPROVE-REQUIRED------------------------*/
    *len = src->buf_len + src->ANCOUNT * 16;
    char *dest = (char *)malloc(*len + 1);

    /* Set Basic Dest Info */
    memcpy(dest, src->req_buf, src->buf_len);
    memcpy(dest + 2, &flag, sizeof(uint16_t));
    //if(src->ANCOUNT == 0)   return dest;

    /* Set Answer Section */
    uint16_t ancount = htons(src->ANCOUNT);
    memcpy(dest + 6, &ancount, sizeof(uint16_t));    //set ancount

    /* set data section */
    for(int i = 0; i < src->ANCOUNT; i++){
        char *dataPos = dest + (src->buf_len + (i * 16));
        int flags = htons(TYPE_A), time = htonl(0x80), dataLen = SIZE_TYPE_A;
        /* Set NAME poiner */
        memcpy(dataPos, &names[i], sizeof(uint16_t));
        /* Set TYPE */
        memcpy((dataPos + 2), &flags, sizeof(uint16_t));
        /* Set CLASS */
        memcpy((dataPos + 4), &flags, sizeof(uint16_t));
        /* Set TTL */
        memcpy((dataPos + 6), &time, sizeof(uint32_t));
        /* Set RDATA */
        memcpy((dataPos + 12), &resData[i], dataLen);
        /* Set RDATA Length */
        dataLen = htons(dataLen);
        memcpy((dataPos + 10), &dataLen, sizeof(uint16_t));
    }
    return dest;

}


/* packet check */
void PacketCheck(Packet *src){
    printf("--------Packet-Check--------\n");
    if(src == NULL){
        printf("<null-ptr>\n");
    }
    /* Origin Buffer Check */
    BuffCheck(src->req_buf, src->buf_len);

    /* Check Header */
    printf("ID : %d\n", src->ID);

    /* Check FLAGS */
    printf("QR : %d\n", GET_QR(src->FLAGS));
    printf("RD : %d\n", GET_RD(src->FLAGS));
    printf("RA : %d\n", GET_RA(src->FLAGS));
    printf("RCODE : %d\n", GET_RCODE(src->FLAGS));

    /* Check COUNTS */
    printf("QDCOUNT : %d\n", src->QDCOUNT);
    printf("ANCOUNT : %d\n", src->ANCOUNT);

    /* Check Question Section */
    for(int i = 0; i < src->QDCOUNT; i++){
        printf("QNAME-%d : %s\n", i, src->QUESTS[i].QNAME);
        printf("QTYPE-%d : %d\n", i, src->QUESTS[i].QTYPE);
        printf("QCLASS-%d : %d\n", i, src->QUESTS[i].QCLASS);
    }

    /* Check Answer Section */
    for(int i = 0; i < src->ANCOUNT; i++){
        printf("NAMEPTR-%d : %d\n", i, src->ANS[i].NAME);
        printf("TYPE-%d : %d\n", i, src->ANS[i].TYPE);
        printf("CLASS-%d : %d\n", i, src->ANS[i].CLASS);
        printf("TTL-%d : %u\n", i, src->ANS[i].TTL);
        printf("RDLEN-%d : %d\n", i, src->ANS[i].RDLEN);
        printf("RDATA-%d : %s\n", i, src->ANS[i].RDATA);
    }
    printf("---------Check-End----------\n\n");
}

void PacketFree(Packet *src){
    /* free Question Section */
    if(src->QUESTS != NULL){
        for(int i = 0; i < src->QDCOUNT; i++){
            free(src->QUESTS[i].QNAME);
        }
        free(src->QUESTS);
    }
    /* free Answer Section */
    if(src->ANS != NULL){
        for(int i = 0; i < src->ANCOUNT; i++){
            free(src->ANS[i].RDATA);
        }
        free(src->ANS);
    }
    src->req_buf = NULL;

    /* free Struct */
    free(src);
}

void BuffCheck(char *buf, int len){
    if(buf == NULL || len <= 0){
        printf("<none buff>\n");
        return;
    }
    printf("BuffLen : %d\nBuff : ", len);
    for(int i = 0; i < len; i++){
        printf("%02X ", (unsigned char)buf[i]);
    }
    printf("\n");
}