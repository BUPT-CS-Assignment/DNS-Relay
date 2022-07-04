#include <server.h>
#include <console.h>


/**
 * @brief url query for Packet
 * 
 * @param src Packet pointer
 * @param records records table pointer
 * @param num records number
 * @return int query result number
 */
int urlQuery(Packet *src, char ***records, int num){
    src->ANCOUNT = 0;
    src->ANS = NULL;
    int res[src->QDCOUNT][16];
    for(int i = 0; i < src->QDCOUNT; i++){
        /* Found All Matches */
        int found = qnameSearch(src->QUESTS[i].QNAME, &res[i], records, num);

        /* Mismatch */
        if(found == 0 || src->QUESTS[i].QTYPE != TYPE_A){
            src->ANCOUNT = 0;
            return 0;
        }

        /* Realloc For Larger Memory */
        int pos = src->ANCOUNT;
        src->ANCOUNT += found;
        src->ANS = (Answer*)realloc(src->ANS,sizeof(Answer) * src->ANCOUNT);

        /* Load All Match Results */
        for(int j = 0; j < found; j++){
            Answer* temp = (Answer *)malloc(sizeof(Answer) * found);
            src->ANS[pos+j].NAME = 12;
            src->ANS[pos+j].TYPE = src->QUESTS[i].QTYPE;
            src->ANS[pos+j].CLASS = src->QUESTS[i].QCLASS;
            src->ANS[pos+j].TTL = 0x80;
            src->ANS[pos+j].RDLEN = 4;
            src->ANS[pos+j].RDATA = (char *)malloc(64 + 1);
            strcpy(src->ANS[pos+j].RDATA, RECORDS[res[i][j]][1]);
        }
    }
    return src->ANCOUNT;
}



/**
 * @brief Qname query from local records table
 * 
 * @param src query source string
 * @param res query result array
 * @param records records table pointer
 * @param urls_num records number
 * @return int query result number
 */
int qnameSearch(char *src, int *res, char ***records, int urls_num){
    if(src == NULL) return 0;
    int ret = 0;
    for(int i = 0; i < urls_num; i++){
        if(strcmp(src, RECORDS[i][0]) == 0){
            res[ret] = i;
            ret ++;
        }
    }
    return ret;
}
