#include "list.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_A 1 // IPV4
#define TYPE_NS 2
#define TYPE_CNAME 5
#define TYPE_MX 15
#define TYPE_TXT 16
#define TYPE_AAAA 28 // IPV6

#define LRU_CACHE_LENGTH 16
#define LRU_OP_SUCCESS 0
#define LRU_OP_FAILED 1
/**
 * @description:查找表条目的基本数据结构
 */
typedef struct
{
    char* domain_name; //域名
    char* ip;          //字符串形式的IP地址
    uint8_t type;      // IP地址类型
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
int LRU_cache_init(LRU_cache* cache);
int LRU_cache_free(LRU_cache* cache);
int __LRU_list_add(LRU_cache* cache, DNS_entry* entry, DNS_entry* location);
DNS_entry* __LRU_list_find(LRU_cache* cache, const char* domain_name);
int LRU_entry_add(LRU_cache* cache, DNS_entry* entry);
int LRU_cache_find(LRU_cache* cache, DNS_entry* entry);
