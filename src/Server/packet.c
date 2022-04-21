#include <server.h>
#include <console.h>

Packet *PacketParse(char *buf, int len){

    Packet *dest = (Packet *)malloc(sizeof(Packet));
    memset(dest, 0, sizeof(Packet));

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
        buf_pos += 4;
    }

    if(dest->ANCOUNT == 0)    return dest;

    /* Get Answers */
    dest->ANS = (Answer *)malloc(sizeof(Answer) * dest->ANCOUNT);
    for(int i = 0; i < dest->ANCOUNT; i++){

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
    }
    flag = ntohs(flag);
    uint16_t qdcount = src->QDCOUNT;

    /* Answer Section */
    uint16_t names[qdcount + 1];
    names[0] = 0xc00c;  //Pos = 1100000000001100 
    for(int i = 1; i < src->QDCOUNT; i++){
        names[i] = names[i - 1] + strlen(src->QUESTS->QNAME) + 5;
    }
    for(int i = 0; i < src->QDCOUNT; i++){
        names[i] = htons(names[i]);
    }

    /* Resource Data */
    uint32_t resData[qdcount + 1];
    for(int i = 0; i < src->QDCOUNT; i++){
        resData[i] = 0;
        UrlFormat(url[i], &resData[i], TYPE_A);
        resData[i] = htonl(resData[i]);
    }

    /* memory allocated */
    /* ------------------------IMPROVE-REQUIRED------------------------*/
    *len = src->buf_len + qdcount * 16;
    char *dest = (char *)malloc(*len + 1);

    /* set basic dest info */
    memcpy(dest, src->req_buf, src->buf_len);
    memcpy(dest + 2, &flag, sizeof(uint16_t));
    uint16_t ancount = htons(qdcount);
    memcpy(dest + 6, &ancount, sizeof(uint16_t));    //set ancount

    /* set data section */
    for(int i = 0; i < qdcount; i++){
        char *dataPos = dest + (src->buf_len + (i * 16));
        int flags = htons(TYPE_A), time = htonl(0x80), dataLen = SIZE_TYPE_A;
        /* set NAME poiner */
        memcpy(dataPos, &names[i], sizeof(uint16_t));
        /* set TYPE */
        memcpy((dataPos + 2), &flags, sizeof(uint16_t));
        /* set CLASS */
        memcpy((dataPos + 4), &flags, sizeof(uint16_t));
        /* set TTL */
        memcpy((dataPos + 6), &time, sizeof(uint32_t));
        /* set RDATA */
        memcpy((dataPos + 12), &resData[i], dataLen);
        /* set RDATA Length */
        dataLen = htons(dataLen);
        memcpy((dataPos + 10), &dataLen, sizeof(uint16_t));
        
        
    }
    return dest;

}


/*
char *UrlFormat(char *url){
    char *res = (char *)malloc(strlen(url) + 1);
    memset(res, 0, strlen(rul));
    int pos = 0;
    strcpy(res + 1, url);
    // url split //
    char *ptr = strtork(url, '.');
    while(ptr){
        int len = strlen(ptr);
        res[pos] = len;
        pos = pos + len + 1;
        ptr = strtok(NULL, '.');
    }
    return res;
}
*/

/* packet check */
void PacketCheck(Packet *packet){
    printf("--------Packet-Check--------\n");
    if(packet == NULL){
        printf("<null-ptr>\n");
    }
    BuffCheck(packet->req_buf, packet->buf_len);
    printf("ID : %d\nFLAG : %d\nQDCOUNT : %d\n", packet->ID, packet->FLAGS, packet->QDCOUNT);
    for(int i = 0; i < packet->QDCOUNT; i++){
        printf("QNAME-%d : %s\n", i, packet->QUESTS->QNAME);
        printf("QTYPE-%d : %d\n", i, packet->QUESTS->QTYPE);
    }
    printf("---------Check-End----------\n");
}

void PacketFree(Packet *packet){
    if(packet->QUESTS != NULL){
        free(packet->QUESTS);
        packet->QUESTS = NULL;
    }
    if(packet->ANS != NULL){
        free(packet->ANS);
        packet->ANS = NULL;
    }
    packet->req_buf = NULL;
    free(packet);
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