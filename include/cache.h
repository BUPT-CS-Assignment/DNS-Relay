#include "list.h"
#include "main.h"
#include "protocol.h"

#define LRU_CACHE_LENGTH 16
#define LRU_OP_SUCCESS 0
#define LRU_OP_FAILED 1

#define MAX_FOUND LRU_CACHE_LENGTH
#define CACHE_TTL 313 //以秒为单位
/**
 * @description:查找表条目的基本数据结构
 */
typedef struct
{
    char* domain_name;  //域名
    char* ip;           //字符串形式的IP地址
    uint8_t type;       //IP地址类型
    uint16_t addition;  //附加字段
    time_t timestamp;   //时间戳
    mylist_head node;
} DNS_entry;

/**
 * @description: LRU缓存的基本数据结构，LRU现在是大小固定，位置相邻的
 */
typedef struct
{
    DNS_entry list[LRU_CACHE_LENGTH];
    mylist_head head;
    int length;
} LRU_cache;

/*函数声明部分*/
int DNS_entry_set(DNS_entry** ptr, char* name, char* ip, uint32_t ttl,uint8_t type, uint16_t addition);
void DNS_entry_free(DNS_entry* entry);
int LRU_cache_init(LRU_cache** ptr);
int LRU_cache_free(LRU_cache* cache);
int __LRU_list_add(LRU_cache* cache, DNS_entry* entry, DNS_entry* location);
int __LRU_list_del(LRU_cache* cache, DNS_entry* entry);
int LRU_entry_add(LRU_cache* cache, DNS_entry* entry);
int LRU_cache_find(LRU_cache* cache, DNS_entry* query, DNS_entry** result);
