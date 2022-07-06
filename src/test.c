#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
static char *RECORDS[][2] = {
    {"bing.com", "204.79.197.200"},        {"baidu.com", "13.107.21.200"},
    {"bilibili.com", "2620:1ec:c11::300"}, {"bupt.edu.cn", "10.3.9.161"},
    {"noui.cloud", "101.43.201.20"},       {"unique.com", "220.177.198.124"}};
int main() {
  LRU_cache *cache;
  cache = (LRU_cache *)malloc(sizeof(LRU_cache));
  INIT_MY_LIST_HEAD(&cache->head);
  cache->length = 0;

  // printf("cache in main:%d\n", cache);
  // printf("head prev:%d\n", cache->head.prev);
  // printf("head next:%d\n", cache->head.next);
  // if (cache->head.next == &cache->head) {
  //   printf("head back linked\n");
  // } else {
  //   printf("head back not linked\n");
  // }
  // if (cache->head.prev == &cache->head) {
  //   printf("head front linked\n");
  // } else {
  //   printf("head front not linked\n");
  // }

  DNS_entry a;
  a.domain_name = (char *)malloc(BUF_SIZE);
  a.ip = (char *)malloc(BUF_SIZE);
  memcpy(&a.domain_name, &RECORDS[0][0], sizeof(RECORDS[0][0]));
  memcpy(&a.ip, &RECORDS[0][1], sizeof(RECORDS[0][1]));
  a.type = TYPE_A;

  DNS_entry b;
  b.domain_name = (char *)malloc(BUF_SIZE);
  b.ip = (char *)malloc(BUF_SIZE);
  memcpy(&b.domain_name, &RECORDS[1][0], sizeof(RECORDS[1][0]));
  memcpy(&b.ip, &RECORDS[1][1], sizeof(RECORDS[1][1]));
  b.type = TYPE_A;

  DNS_entry c;
  c.domain_name = (char *)malloc(BUF_SIZE);
  c.ip = (char *)malloc(BUF_SIZE);
  memcpy(&c.domain_name, &RECORDS[2][0], sizeof(RECORDS[2][0]));
  memcpy(&c.ip, &RECORDS[2][1], sizeof(RECORDS[2][1]));
  c.type = TYPE_AAAA;

  DNS_entry d;
  d.domain_name = (char *)malloc(BUF_SIZE);
  d.ip = (char *)malloc(BUF_SIZE);
  memcpy(&d.domain_name, &RECORDS[3][0], sizeof(RECORDS[3][0]));
  memcpy(&d.ip, &RECORDS[3][1], sizeof(RECORDS[3][1]));
  d.type = TYPE_A;

  DNS_entry e;
  e.domain_name = (char *)malloc(BUF_SIZE);
  e.ip = (char *)malloc(BUF_SIZE);
  memcpy(&e.domain_name, &RECORDS[4][0], sizeof(RECORDS[4][0]));
  memcpy(&e.ip, &RECORDS[4][1], sizeof(RECORDS[4][1]));
  e.type = TYPE_A;

  DNS_entry f;
  f.domain_name = (char *)malloc(BUF_SIZE);
  f.ip = (char *)malloc(BUF_SIZE);
  memcpy(&f.domain_name, &RECORDS[5][0], sizeof(RECORDS[5][0]));
  memcpy(&f.ip, &RECORDS[5][1], sizeof(RECORDS[5][1]));
  f.type = TYPE_A;
  LRU_entry_add(&cache, &a);
  LRU_entry_add(&cache, &b);
  LRU_entry_add(&cache, &c);
  LRU_entry_add(&cache, &d);

  // if (cache->list[0].node.next == &cache->head) {
  //   printf("linked\n");
  // } else {
  //   printf("not linked\n");
  // }
  printf("length:%d\n", cache->length);
  mylist_head *p;
  mylist_for_each(p, &cache->head) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s--", entry->domain_name);
    printf("ip:%s--", entry->ip);
    printf("type:%d\n\n", entry->type);
  }

  LRU_entry_add(&cache, &e);
  mylist_for_each(p, &cache->head) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s--", entry->domain_name);
    printf("ip:%s--", entry->ip);
    printf("type:%d\n\n", entry->type);
  }

  LRU_entry_add(&cache, &f);

  mylist_for_each(p, &cache->head) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s--", entry->domain_name);
    printf("ip:%s--", entry->ip);
    printf("type:%d\n\n", entry->type);
  }

  printf("length:%d\n", cache->length);

  return 0;
}