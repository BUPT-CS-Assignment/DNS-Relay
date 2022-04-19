#include <server.h>
#include <console.h>

Packet *RequestParse(char *buf, int len){
    Packet *packet = (Packet *)malloc(sizeof(Packet) + 1);
    memset(packet, 0, sizeof(Packet));
    /* set req_buf */
    packet->req_buf = buf;
    packet->buf_len = len;

    /* set Header */
    memcpy(&packet->ID, buf, sizeof(uint16_t));
    memcpy(&packet->FLAGS, buf + 2, sizeof(uint16_t));
    packet->QDCOUNT = ntohs(*((uint16_t *)(buf + 4)));
    //packet->ANCOUNT = ntohs(*((uint16_t *)(buf + 6)));
    /* Set Question Number */
    char *pos = buf + 12;
    packet->QUESTS = (Quest *)malloc(sizeof(Quest) * packet->QDCOUNT);
    /* Get Questions */
    for(int i = 0; i < packet->QDCOUNT; i++){
        /* read till '\0' */
        int q_len = strlen(pos);
        /* QNAME　parse */
        packet->QUESTS[i].QNAME = UrlParse(pos);
        /* QTYPE parse */
        pos += (q_len + 1);
        packet->QUESTS[i].QTYPE = ntohs(*((uint16_t *)pos));
        pos += 4;
    }
    return packet;
}

char *UrlParse(char *buf){
    char *res = (char *)malloc(strlen(buf) + 1);
    int pos = 0;
    while(buf[pos] != '\0'){
        /* length byte */
        int len = buf[pos];
        /* read chars */
        len = (len > 64 ? 64 : len);
        for(int i = 0; i < len; i++){
            res[pos + i] = buf[pos + i + 1];
        }
        res[pos + len] = '.';   //add '.'
        pos += (len + 1);   //pointer move
    }
    res[pos - 1] = '\0';
    return res;
}


char *ResponseFormat(Packet packet, int *len, char **url){
    /* Header Section */
    //char header[12];
    //memset(header, 0, 12);
    //memcpy(header, &packet.ID, sizeof(uint16_t));

    /* Set Flags */
    uint16_t flag = packet.FLAGS | 0x8180; //QR = 1, RD = 1, AA = 1
    flag = (packet.QUESTS[0].QTYPE==1? htons(0x8580) : htons(0x8583));
    //memcpy(header + 2, &packet.FLAGS, sizeof(uint16_t));
    uint16_t qdcount = packet.QDCOUNT;
    /* Set QDCOUNT, ANCOUNT */
    //memcpy(header + 4, &qdcount, sizeof(uint16_t));
    //memcpy(header + 6, &qdcount, sizeof(uint16_t));

    /* Question Section */

    /* Answer Section */
    uint16_t names[qdcount + 1];
    names[0] = htons(0xc00c);  //Pos = 1100000000001100 
    for(int i = 1; i < packet.QDCOUNT; i++){
        names[i] = names[i - 1] + strlen(packet.QUESTS->QNAME) + 5;
    }
    /* Resource Data */
    uint32_t resData[qdcount + 1];
    for(int i = 0; i < packet.QDCOUNT; i++){
        resData[i] = htonl(UrlFormat(url[i]));
    }
    *len = packet.buf_len + qdcount * 16;
    char *res = (char *)malloc(*len + 1);
    memcpy(res, packet.req_buf, packet.buf_len);
    memcpy(res + 2, &flag, sizeof(uint16_t));   //set new flag
    uint16_t ancount = htons(qdcount);
    memcpy(res + 6, &ancount, sizeof(uint16_t));    //set ancount
    for(int i = 0; i < qdcount; i++){
        char *dataPos = res + (packet.buf_len + (i * 16));
        int flags = htons(0x01), time = htonl(0x80), dataLen = htons(0x04);
        memcpy(dataPos, &names[i], sizeof(uint16_t));
        memcpy((dataPos + 2), &flags, sizeof(uint16_t));
        memcpy((dataPos + 4), &flags, sizeof(uint16_t));
        memcpy((dataPos + 6), &time, sizeof(uint32_t));
        memcpy((dataPos + 10), &dataLen, sizeof(uint16_t));
        memcpy((dataPos + 12), &resData[i], sizeof(uint32_t));
    }
    return res;

}

uint32_t UrlFormat(char *url){
    uint32_t res = 0;
    char temp[strlen(url) + 1];
    strcpy(temp, url);
    char *ptr = strtok(temp, ".");
    for(int i = 0; i < 4; i++){
        res |= (atoi(ptr) << ((3 - i) * 8));
        ptr = strtok(NULL, ".");
    }
    return res;
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