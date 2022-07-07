#include <server.h>
#include <console.h>


int urlStore(Packet* src)
{
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        DNS_entry* entry;
        Answer* pANS = &src->ANS[i];
        DNS_entry_set(&entry, src->QUESTS[pANS->QPOS].QNAME, pANS->RDATA, pANS->TTL, pANS->TYPE, pANS->ADDITION);
        LRU_entry_add(_url_cache, entry);
    }
    return _url_cache->length;
}


/**
 * @brief url query for Packet
 *
 * @param src Packet pointer
 * @param records records table pointer
 * @param num records number
 * @return int query result number
 */
int urlQuery(Packet* src)
{
    src->ANCOUNT = 0;
    src->ANS = NULL;
    int res[src->QDCOUNT][16];
    for(int i = 0; i < src->QDCOUNT; i++)
    {
        /* Found All Matches */
        DNS_entry* result;
        int found = qnameSearch(src->QUESTS[i].QNAME, src->QUESTS[i].QTYPE, &result);
        /* Mismatch */
        if(found == 0)
        {
            src->ANCOUNT = 0;
            return 0;
        }

        /* Realloc For Larger Memory */
        int pos = src->ANCOUNT;
        src->ANCOUNT += found;
        src->ANS = (Answer*)realloc(src->ANS, sizeof(Answer) * src->ANCOUNT);

        /* Load All Match Results */
        for(int j = 0; j < found; j++)
        {
            Answer* temp = (Answer*)malloc(sizeof(Answer) * found);
            src->ANS[pos + j].QPOS = i;
            src->ANS[pos + j].TYPE = src->QUESTS[i].QTYPE;
            src->ANS[pos + j].CLASS = src->QUESTS[i].QCLASS;
            src->ANS[pos + j].TTL = result[j].timestamp - time(NULL);
            src->ANS[pos + j].ADDITION = result[j].addition;
            src->ANS[pos + j].RDATA = (char*)malloc(TYPE_BUF_SIZE(src->QUESTS[i].QTYPE));
            strcpy(src->ANS[pos + j].RDATA, result[j].ip);
            DNS_entry_free(&result[j]);

        }
        free(result);
    }
    return src->ANCOUNT;
}



/**
 * @brief Qname query from local records table
 *
 * @param src query source string
 * @param type query address type
 * @param res query result array
 * @param records records table pointer
 * @param urls_num records number
 * @return int query result number
 */
int qnameSearch(char* qname, uint16_t qtype, DNS_entry** result)
{
    if(qname == NULL) return 0;

    DNS_entry* _entry;
    DNS_entry_set(&_entry, qname, NULL, 0, qtype, 0);
    int ret = LRU_cache_find(_url_cache, _entry, result);
    consoleLog(DEBUG_L0, BOLDGREEN"> query from cache return %d\n", ret);
    DNS_entry_free(_entry);
    free(_entry);
    return ret;
}
