#include "server.h"
#include "console.h"


int urlStore(Packet* src)
{
    consoleLog(DEBUG_L1, BOLDMAGENTA"> store into cache\n");
    //char name[255];
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        if(src->ANS[i].TYPE != src->QUESTS[0].QTYPE)
        {
            continue;
        }
        DNS_entry* entry;
        Answer* pANS = &src->ANS[i];

        //urlParse(&pANS->NAME,name,NULL,TYPE_CNAME,2,src->req_buf);
        //DNS_entry_set(&entry, name, pANS->RDATA, pANS->TTL, pANS->TYPE, pANS->ADDITION);

        DNS_entry_set(&entry, src->QUESTS[0].QNAME, pANS->RDATA, pANS->TTL, pANS->TYPE, pANS->ADDITION);
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
    for(int i = 0; i < src->QDCOUNT; i++)
    {
        /* Found All Matches */
        DNS_entry* result = NULL;
        int resource;
        int found = qnameSearch(src->QUESTS[i].QNAME, src->QUESTS[i].QTYPE, &result, &resource);
        /* Mismatch */
        if(found == -1){
            src->ANCOUNT = 0;
            free(result);
            return -1;
        }
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
            src->ANS[pos + j].QPOS = i;
            src->ANS[pos + j].TYPE = src->QUESTS[i].QTYPE;
            src->ANS[pos + j].CLASS = src->QUESTS[i].QCLASS;
            src->ANS[pos + j].TTL = result[j].timestamp - time(NULL);
            src->ANS[pos + j].ADDITION = result[j].addition;
            src->ANS[pos + j].RDATA = (char*)malloc(TYPE_BUF_SIZE(src->QUESTS[i].QTYPE));
            strcpy(src->ANS[pos + j].RDATA, result[j].ip);
            if(resource == RESOURCE_CACHE)
            {
                DNS_entry_free(&result[j]);
            }
        }
        free(result);
    }
    return src->ANCOUNT;
}


void* file_find_handler(void* args);


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
int qnameSearch(char* qname, uint16_t qtype, DNS_entry** result, int* resource)
{
    if(qname == NULL) return 0;

    DNS_entry* _entry;
    DNS_entry_set(&_entry, qname, NULL, 0, qtype, 0);

    DNS_entry* res_1 = NULL, * res_2 = NULL;
    int* flag = (int*)malloc(sizeof(int));
    int ret_2 = 0;
    *flag = 0;

    int ret = LRU_cache_find(_url_cache, _entry, &res_1);

    void* args[] = {&_hash_map,_entry,&res_2,&ret_2,flag};
    thread_t t_num = threadCreate((void*)file_find_handler, args);
    void* local_found;
    if(ret == 0)
    {
        threadJoin(t_num, NULL);
        consoleLog(DEBUG_L0, BOLDGREEN"> query from host return %d\n", ret_2);
        if(ret_2 != 0)
        {
            *result = res_2;
        }
        ret = ret_2;
        *resource = RESOURCE_HOST;
    }
    else
    {
        threadDetach(t_num);
        consoleLog(DEBUG_L0, BOLDGREEN"> query from cache return %d\n", ret);
        *result = res_1;
        *resource = RESOURCE_CACHE;

    }

    DNS_entry_free(_entry);
    free(_entry);
    return ret;
}


void* file_find_handler(void* args)
{
    hash* map = (hash*)((void**)args)[0];
    DNS_entry* temp = (DNS_entry*)((void**)args)[1];
    DNS_entry** result = (DNS_entry**)((void**)args)[2];
    int* ret = (int*)((void**)args)[3];
    int* flag = (int*)((void**)args)[4];
    DNS_entry* entry;
    DNS_entry_set(&entry, temp->domain_name, temp->ip, 0, temp->type, temp->addition);
    *ret = file_find(entry, result, map);
    DNS_entry_free(entry);
    free(entry);
    threadExit(1);
}
