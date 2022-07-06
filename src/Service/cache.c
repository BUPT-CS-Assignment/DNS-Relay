#include "cache.h"
#include <stdlib.h>
#include <string.h>

/**
 * @description: 初始化缓存单元，动态分配内存
 * @param {LRU_cache} *cache 提前声明好，要被初始化的缓存变量
 * @return {*}
 */
int LRU_cache_init(LRU_cache **cptr) {
  *cptr = (LRU_cache *)malloc(sizeof(LRU_cache));
  INIT_MY_LIST_HEAD(&(*cptr)->head);
  (*cptr)->length = 0;
  return LRU_OP_SUCCESS;
}

/**
 * @description: 析构缓存单元，释放动态分配好的内存
 * @param {LRU_cache} *cache
 * @return {*}
 */
int LRU_cache_free(LRU_cache **cptr) {
  LRU_cache *cache = *cptr;
  for (int i = 0; i < cache->length; i++) {
    free(cache->list[i].domain_name);
    free(cache->list[i].ip);
  }
  free(cache);
  return LRU_OP_SUCCESS;
}

/**
 * @description: 查找链表中所有符合特定域名与特定类型的条文
 * @param {LRU_cache} *
 * @param {DNS_entry*} query 要查询的含有对应域名及类型的条文
 * @param {DNS_entry*} result
 * 查询到的所有条文，为动态分配的指针，对应的字符串均被深拷贝过，释放靠上层
 * @return {*} 返回值为查询到符合的条文数量
 */
int LRU_cache_find(LRU_cache **cptr, DNS_entry *query, DNS_entry *result) {
  LRU_cache *cache = *cptr;
  int count = 0;
  result = (DNS_entry *)malloc(sizeof(DNS_entry[LRU_CACHE_LENGTH]));
  mylist_head *p;
  mylist_for_each(p, &cache->head) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    if (strcmp(entry->domain_name, query->domain_name) == 0) {
      if (entry->type == query->type) {
        result[count].domain_name = (char *)malloc(sizeof(entry->domain_name));
        memcpy(result[count].domain_name, entry->domain_name,
               strlen(entry->domain_name) + 1);
        result[count].ip = (char *)malloc(sizeof(entry->ip));
        memcpy(result[count].ip, entry->ip, strlen(entry->ip) + 1);
        result[count].type = entry->type;
        count++;
        mylist_rotate_node_head(&entry->node, &cache->head);
      }
    }
  }
  return count;
}

/**
 * @description: 向条文链表中加入新一条，写入指定的内存位置中，为内部使用的函数
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry 要被加入的新条文
 * @param {DNS_entry} *location 指定好的指定内存位置
 * @return {*}
 */
int __LRU_list_add(LRU_cache **cptr, DNS_entry *entry, DNS_entry *location) {
  LRU_cache *cache = *cptr;
  memcpy(location, entry, sizeof(DNS_entry));
  location->domain_name = (char *)malloc(sizeof(entry->domain_name));
  location->ip = (char *)malloc(sizeof(entry->ip));
  memcpy(location->domain_name, entry->domain_name,
         strlen(entry->domain_name) + 1);
  memcpy(location->ip, entry->ip, strlen(entry->ip) + 1);
  mylist_add_head(&location->node, &cache->head);

  return LRU_OP_SUCCESS;
}
/**
 * @description: 删除链表中的指定一个节点，为内部使用的函数
 * @param {LRU_cache} *
 * @param {DNS_entry} *entry
 * @return {*}
 */
int __LRU_list_del(LRU_cache **cptr, DNS_entry *entry) {
  LRU_cache *cache = *cptr;
  mylist_del_init(&entry->node);

  return LRU_OP_SUCCESS;
}
/**
 * @description: 将互联网上查到的新报文加入缓存系统中，为外部调用的接口
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry
 * 互联网上查询到的内容组织成的新报文，事先应封装好对应内容
 * @return {*}
 */
int LRU_entry_add(LRU_cache **cptr, DNS_entry *entry) {
  LRU_cache *cache = *cptr;
  if (cache->length <
      LRU_CACHE_LENGTH) { //如果缓存空间仍未满，直接在内存空闲位置加入新条文，新条文会位于链表头
    __LRU_list_add(&cache, entry, &cache->list[cache->length]);
    cache->length++;
  } else { //如果缓存空间位置已满，将链表最后的一条文的内存位置腾出给新条文，新条文会位于链表头
    DNS_entry *tail = mylist_entry(cache->head.prev, DNS_entry, node);
    // printf("tail->dn:%s\n",tail->domain_name);
    if (mylist_is_last(&tail->node, &cache->head)) {
      __LRU_list_del(&cache, tail);
      free(tail->domain_name);
      free(tail->ip);
      __LRU_list_add(&cache, entry, tail);
    }
  }
  return LRU_OP_SUCCESS;
}
