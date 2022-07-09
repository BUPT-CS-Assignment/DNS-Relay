#ifndef __CACHE__
#define __CACHE__

#include "main.h"
#include "protocol.h"
#include "utils/hash.h"
#include "utils/list.h"
#include <stdint.h>

extern int LRU_CACHE_LENGTH;

#define LRU_OP_SUCCESS 0
#define LRU_OP_FAILED 1

/**
 * @description:查找表条目的基本数据结构
 */
typedef struct {
  char *domain_name; //域名
  char *ip;          //字符串形式的IP地址
  uint8_t type;      // IP地址类型
  uint16_t addition; //附加字段
  time_t timestamp;  //时间戳
  mylist_head node;
} DNS_entry;

/**
 * @description: LRU缓存的基本数据结构，LRU现在是大小固定，位置相邻的
 */
typedef struct {
  DNS_entry *list;
  mylist_head head;
  int length;
  uint8_t *set;
  rwlock_t lock;
} LRU_cache;

typedef struct {
  DNS_entry *Dptr;
  mylist_head node;
} Dptr_in_map;

/*函数声明部分*/
int DNS_entry_set(DNS_entry **ptr, char *name, char *ip, uint32_t ttl,
                  uint8_t type, uint16_t addition);
void DNS_entry_free(DNS_entry *entry);
int LRU_cache_init(LRU_cache **ptr);
int LRU_cache_free(LRU_cache *cache);
int LRU_entry_add(LRU_cache *cache, DNS_entry *entry);
int LRU_cache_find(LRU_cache *cache, DNS_entry *query, DNS_entry **result);
void LRU_cache_check(LRU_cache *cache);
int LRU_cache_clean(LRU_cache *cache);

int insert_one(struct string_hash *map, mylist_head **head, Dptr_in_map *dp);
int query_list(struct string_hash *map, Dptr_in_map *dp, mylist_head **result);
int query_one(struct string_hash *map, Dptr_in_map *dp, Dptr_in_map **found);
int delete_one(struct string_hash *map, Dptr_in_map *dp);
#endif