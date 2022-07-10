#include "server.h"
#include "console.h"

/* --------------------------------- Basic Definition ---------------------------------*/
/**
 * @brief host-query thread args
 * 
 */
typedef struct hq_args
{
    DNS_entry* entry;
    DNS_entry** res_dest;
    int* res_num;
    int* flag;

}hq_args;

void* host_query_handle(void* args);    //host-query thread handler


/* --------------------------------- Main Function ---------------------------------*/

/**
 * @brief store record into cache
 * 
 * @param src packet pointer
 * @return int current cache length
 */
int urlStore(Packet* src)
{
    consoleLog(DEBUG_L1, BOLDMAGENTA"> store into cache\n");
    for(int i = 0; i < src->ANCOUNT; i++)
    {
        if(src->ANS[i].TYPE != src->QUESTS[0].QTYPE)    //not query type, cancel store
        {
            continue;
        }

        DNS_entry* entry;
        Answer* pANS = &src->ANS[i];

        /* fill dns_entry && insert into cache */
        DNS_entry_set(&entry, src->QUESTS[0].QNAME, pANS->RDATA, pANS->TTL, pANS->TYPE, pANS->ADDITION);
        cacheInsert(__URL_CACHE__, entry);
    }
    return __URL_CACHE__->length;
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

        if(found == -1)
        {
            /* qname in black list */
            src->ANCOUNT = 0;
            free(result);
            return -1;
        }
        if(found == 0)
        {
            /* Mismatch */
            src->ANCOUNT = 0;
            free(result);
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
            strcpy(src->ANS[pos + j].RDATA, result[j].ip);      //copy result data

            if(resource == RESOURCE_CACHE)  DNS_entry_free(&result[j]);     //results from host are not deep-copied
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
int qnameSearch(char* qname, uint16_t qtype, DNS_entry** result, int* resource)
{
    if(qname == NULL) return 0;

    /* construct dns_entry for query */
    DNS_entry* entry_cache, * entry_host;
    DNS_entry_set(&entry_cache, qname, NULL, 0, qtype, 0);
    DNS_entry_set(&entry_host, qname, NULL, 0, qtype, 0);

    DNS_entry* res_cache = NULL, * res_host = NULL;     //query result dest from cache/host
    int ret_cache = 0, ret_host = 0;                    //query result number from cache/host
    int* host_flag = (int*)malloc(sizeof(int));         //query stop flag for host
    *host_flag = 1;
    
    /* fill hash-query thread args */
    hq_args* hq_arg = (hq_args*)malloc(sizeof(hq_args));
    hq_arg->entry = entry_host;
    hq_arg->res_dest = &res_host;
    hq_arg->res_num = &ret_host;
    hq_arg->flag = host_flag;

    /* start thread for query host && query cache */
    thread_t thread_host = threadCreate((void*)host_query_handle, hq_arg);
    ret_cache = cacheQuery(__URL_CACHE__, entry_cache, &res_cache);

    int ret;
    if(ret_cache == 0)
    {
        /* not in cache, wait for query-host thread */
        threadJoin(thread_host, NULL);
        consoleLog(DEBUG_L0, BOLDGREEN"> query from host return %d\n", ret_host);
        if(ret_host != 0)   *result = res_host;     //query success from host (res number > 0)
        ret = ret_host;
        *resource = RESOURCE_HOST;
    }
    else
    {
        /* in cache, stop host-query thread */
        threadDetach(thread_host);
        *host_flag = 0;
        consoleLog(DEBUG_L0, BOLDGREEN"> query from cache return %d\n", ret_cache);
        *result = res_cache;                        //query success from cache (res number > 0)
        ret = ret_cache;
        *resource = RESOURCE_CACHE;

    }

    /* free memory */
    DNS_entry_free(entry_cache);
    free(entry_cache);
    return ret;
}



/* --------------------------------- Thread Function ---------------------------------*/

/**
 * @brief host query thread handler
 * 
 * @param args thread args hq_args
 * @return void* 
 */
void* host_query_handle(void* args)
{
    hq_args* arg = (hq_args*)args;

    /* queyr from host */
    *(*arg).res_num = hostQuery((*arg).entry, (*arg).res_dest, &__HOST_HASHMAP__, (*arg).flag);

    /* free memory */
    DNS_entry_free((*arg).entry);
    free((*arg).entry);
    free(arg);

    threadExit(1);
}
