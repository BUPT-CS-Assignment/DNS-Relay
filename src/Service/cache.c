#include "console.h"
#include "host.h"
#include "cache.h"

// extern hash *map;

/* --------------------------------- Basic Definition ---------------------------------*/
/**
 * @brief rotate thread args
 *
 */
typedef struct cache_args
{
    LRU_cache* cache;
    DNS_entry** temp_array;
    int count;
} cache_args;

int __CACHE_LEN__ = 64;

/* --------------------------------- Entry Setting ---------------------------------*/

/**
 * @brief fill dns_entry structure
 *
 * @param ptr dns_entry pointer address
 * @param name domain_name
 * @param ip ip/cname/ns/mx records
 * @param ttl timeout record
 * @param type record type
 * @param addition addtional record (preference on MX)
 * @return int fill result
 */
int DNS_entry_set(DNS_entry** ptr, char* name, char* ip, uint32_t ttl,
    uint8_t type, uint16_t addition)
{
    if(ptr == NULL)
        return -1;
    *ptr = (DNS_entry*)malloc(sizeof(DNS_entry)); // apply for memory space

    if(name != NULL)
    {
        (*ptr)->domain_name = (char*)malloc(strlen(name) + 1); // copy domain_name
        strcpy((*ptr)->domain_name, name);
    }
    else
    {
        (*ptr)->domain_name = NULL;
    }
    if(ip != NULL)
    {
        (*ptr)->ip = (char*)malloc(strlen(ip) + 1); // copy ip/cname/mx/ns reocrd
        strcpy((*ptr)->ip, ip);
    }
    else
    {
        (*ptr)->ip = NULL;
    }
    (*ptr)->type = type;                  // copy type section
    (*ptr)->addition = addition;          // copy addition section
    (*ptr)->timestamp = time(NULL) + ttl; // set timestamp
    return 0;
}



/**
 * @brief free dns_entry structure
 *
 * @param entry dns_entry struct pointer
 */
void DNS_entry_free(DNS_entry* entry)
{
    if(entry == NULL)
        return;
    free(entry->domain_name);
    entry->domain_name = NULL;
    free(entry->ip);
    entry->ip = NULL;
}


/* --------------------------------- Inner LRU_cache Function ---------------------------------*/
 /**
  * @description: 向条文链表中加入新一条，写入指定的内存位置中，为内部使用的函数
  * @param {LRU_cache} *cache
  * @param {DNS_entry} *entry 要被加入的新条文
  * @param {DNS_entry} *location 指定好的指定内存位置
  * @return {*}
  */
int __LRU_list_add(LRU_cache* cache, DNS_entry* entry, DNS_entry* location)
{
    memcpy(location, entry, sizeof(DNS_entry));
    if(entry->domain_name != NULL)
    {
        location->domain_name = (char*)malloc(strlen(entry->domain_name) + 1);
        strcpy(location->domain_name, entry->domain_name);
    }
    else
    {
        location->domain_name = NULL;
    }
    if(entry->ip != NULL)
    {
        location->ip = (char*)malloc(strlen(entry->ip) + 1);
        strcpy(location->ip, entry->ip);
    }
    else
    {
        location->ip = NULL;
    }

    mylist_add_head(&location->node, &cache->head);

    return LRU_OP_SUCCESS;
}



/**
 * @description: 删除链表中的指定一个节点，为内部使用的函数
 * @param {LRU_cache} *
 * @param {DNS_entry} *entry
 * @return {*}
 */
int __LRU_list_del(LRU_cache* cache, DNS_entry* entry)
{
    free(entry->domain_name);
    entry->domain_name = NULL;
    free(entry->ip);
    entry->ip = NULL;
    mylist_del(&entry->node);
    return LRU_OP_SUCCESS;
}



/**
 * @brief multi-thread LRU_cache rotate
 *
 * @param param thread param pointer (normally cache_args)
 * @return void*
 */
void* __LRU_cache_rotate(void* param)
{
    cache_args* args = (cache_args*)param;

    /* param copy　*/
    LRU_cache* cache = args->cache;
    DNS_entry** temp = args->temp_array;

    writeLock(&(cache->lock)); // set lock
    for(int i = 0; i < args->count; i++)
    {
        mylist_rotate_node_head(&temp[i]->node, &cache->head);
    }
    unlock(&(cache->lock)); // unlock

    free(temp);
    free(args);
    threadExit(0);
}



/* --------------------------------- Usage Function ---------------------------------*/

 /**
  * @description: 初始化缓存单元，动态分配内存
  * @param {LRU_cache} *cache 提前声明好，要被初始化的缓存变量
  * @return {*}
  */
int cacheInit(LRU_cache** cptr)
{
    *cptr = (LRU_cache*)malloc(sizeof(LRU_cache));
    (*cptr)->list = (DNS_entry*)malloc(sizeof(DNS_entry) * __CACHE_LEN__);
    (*cptr)->set = malloc(sizeof(uint8_t) * __CACHE_LEN__);
    memset((*cptr)->set, 0, sizeof(uint8_t) * __CACHE_LEN__);
    INIT_MY_LIST_HEAD(&(*cptr)->head);

    (*cptr)->length = 0;
    lockInit(&(*cptr)->lock);
    return LRU_OP_SUCCESS;
}



/**
 * @description: 析构缓存单元，释放动态分配好的内存
 * @param {LRU_cache} *cache
 * @return {*}
 */
int cacheFree(LRU_cache* cache)
{
    writeLock(&cache->lock);
    for(int i = 0; i < cache->length; i++)
    {
        free(cache->list[i].domain_name);
        free(cache->list[i].ip);
    }
    lockDestroy(&(cache->lock));
    return LRU_OP_SUCCESS;
}



int cacheOutput(LRU_cache* cache)
{
    printf(BOLDMAGENTA"> cache records output.\n");
    FILE* fp = fopen("cache.txt", "w");
    if(fp == NULL)
    {
        printf(BOLDRED"> file create failed.\n");
        return -1;
    }
    mylist_head* p;
    fprintf(fp,"*** Cache Records ***\n");

    readLock(&(cache->lock));
    fprintf(fp,"Total Length: %d\n\n", cache->length);
    mylist_for_each(p, &cache->head)
    {
        DNS_entry* entry = mylist_entry(p, DNS_entry, node);
        fprintf(fp,"  %s\n",entry->domain_name);
        fprintf(fp,"  --------------------------\n");
        fprintf(fp,"  RR. . . . . : %s\n", entry->ip);
        fprintf(fp,"  type. . . . : %d\n", entry->type);
        fprintf(fp,"  ttl . . . . : %d\n", entry->timestamp - time(NULL));
        if(entry->type == TYPE_MX)
        {
            fprintf(fp,"  pref. . . . : %d\n", entry->addition);
        }
        fprintf(fp,"\n");
    }
    
    unlock(&(cache->lock));
    fclose(fp);
    printf(BOLDGREEN "> output ok. check 'cache.txt'.\n");
    return 0;
}


void cacheCheck(LRU_cache* cache)
{
    mylist_head* p;
    printf(BOLDMAGENTA "> cache check\n" RESET);
    readLock(&(cache->lock));

    mylist_for_each(p, &cache->head)
    {
        DNS_entry* entry = mylist_entry(p, DNS_entry, node);
        printf("  %s\n",entry->domain_name);
        printf("  ---------------------------\n");
        printf("  RR. . . . . : %s\n", entry->ip);
        printf("  type. . . . : %d\n", entry->type);
        printf("  ttl . . . . : %d\n", entry->timestamp - time(NULL));
        if(entry->type == TYPE_MX)
        {
            printf("  pref. . . . : %d\n", entry->addition);
        }
        printf("\n");
    }
    unlock(&(cache->lock));
    printf(BOLDBLUE"Total Length: %d\n"RESET, cache->length);
    printf(BOLDGREEN "> check end\n"RESET);
}



/**
 * @description: 查找链表中所有符合特定域名与特定类型的条文
 * @param {LRU_cache} *
 * @param {DNS_entry*} query 要查询的含有对应域名及类型的条文
 * @param {DNS_entry*} result
 * 查询到的所有条文，为动态分配的指针，对应的字符串未被深拷贝过，释放靠上层
 * @return {*} 返回值为查询到符合的条文数量
 */
int cacheQuery(LRU_cache* cache, DNS_entry* query, DNS_entry** result)
{
    int count = 0;
    *result = (DNS_entry*)malloc(sizeof(DNS_entry) * __CACHE_LEN__);
    DNS_entry** temp =
        (DNS_entry**)malloc(sizeof(DNS_entry*) * __CACHE_LEN__);
    mylist_head* p;
    readLock(&(cache->lock));
    mylist_for_each(p, &cache->head)
    {
        DNS_entry* entry = mylist_entry(p, DNS_entry, node);
        if(strcmp(entry->domain_name, query->domain_name) == 0)
        {
            if(entry->type == query->type)
            {
                (*result)[count].domain_name = (char*)malloc(strlen(entry->domain_name) + 1);
                strcpy((*result)[count].domain_name, entry->domain_name);
                (*result)[count].ip = (char*)malloc(strlen(entry->ip) + 1);
                strcpy((*result)[count].ip, entry->ip);
                (*result)[count].type = entry->type;
                (*result)[count].timestamp = entry->timestamp;
                (*result)[count].addition = entry->addition;
                temp[count] = entry;
                count++;
            }
        }
    }
    unlock(&(cache->lock));

    /* fill cache-rotate thread args */
    cache_args* cargs = (cache_args*)malloc(sizeof(cache_args));
    cargs->cache = cache;
    cargs->count = count;
    cargs->temp_array = temp;

    /* start && detach thread */
    threadDetach(threadCreate(__LRU_cache_rotate, cargs));

    return count;
}



int cacheScan(LRU_cache* cache)
{
    writeLock(&(cache->lock));
    int flag = 0;
    mylist_head* p;
    mylist_for_each(p, &cache->head)
    {
        DNS_entry* entry = mylist_entry(p, DNS_entry, node);
        if(entry->timestamp < time(NULL))
        {
            consoleLog(DEBUG_L1, BOLDYELLOW "> cache record overdue.\n");
            p = p->prev;
            __LRU_list_del(cache, entry);
            int offset = entry - cache->list;
            cache->set[offset] = 0;
            cache->length--;
            flag = 1;
        }
    }
    unlock(&(cache->lock));
    return flag;
}



/**
 * @description: 将互联网上查到的新报文加入缓存系统中，为外部调用的接口
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry
 * 互联网上查询到的内容组织成的新报文，事先应封装好对应内容
 * @return {*}
 */
int cacheInsert(LRU_cache* cache, DNS_entry* entry)
{
    writeLock(&(cache->lock));
    if(cache->length < __CACHE_LEN__)
    {
        int i;
        for(i = 0; i < __CACHE_LEN__; i++)
        {
            if(cache->set[i] == 0)  break;
        }
        __LRU_list_add(cache, entry, &cache->list[i]);
        mylist_head* head;
        cache->set[i] = 1;
        cache->length++;
    }
    else
    {
        //如果缓存空间位置已满，将链表最后的一条文的内存位置腾出给新条文，新条文会位于链表头
        DNS_entry* tail = mylist_entry(cache->head.prev, DNS_entry, node);
        if(mylist_is_last(&tail->node, &cache->head))
        {
            __LRU_list_del(cache, tail);
            __LRU_list_add(cache, entry, tail);
        }
    }
    unlock(&(cache->lock));

    return LRU_OP_SUCCESS;
}



int cacheFlush(LRU_cache* cache)
{
    writeLock(&(cache->lock));
    for(int i = 0; i < cache->length; i++)
    {
        free(cache->list[i].domain_name);
        free(cache->list[i].ip);
        cache->list[i].domain_name = NULL;
        cache->list[i].ip = NULL;
        mylist_del(&cache->list[i].node);
    }
    memset(cache->set, 0, sizeof(uint8_t) * __CACHE_LEN__);
    INIT_MY_LIST_HEAD(&cache->head);
    cache->length = 0;
    unlock(&(cache->lock));
    consoleLog(DEBUG_L1, BOLDGREEN "> cache flush ok.\n");
    return 0;

}
