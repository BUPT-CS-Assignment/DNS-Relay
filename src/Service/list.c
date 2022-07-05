#include "list.h"
#include <stdint.h>
#include <stdio.h>
/**
 * @description:查找表条目的基本数据结构
 */
typedef struct {
  char *domain_name; //域名
  char *ip;          //字符串形式的IP地址
  uint8_t type;      // IP地址类型
  mylist_head node;
} DNS_entry;

#define TYPE_A 1//IPV4
#define TYPE_NS 2
#define TYPE_CNAME 5
#define TYPE_MX 15
#define TYPE_TXT 16
#define TYPE_AAAA 28//IPV6

static char *RECORDS[][2] = {
    {"bing.com", "204.79.197.200"},    {"bing.com", "13.107.21.200"},
    {"bing.com", "2620:1ec:c11::300"}, {"bupt.edu.cn", "10.3.9.161"},
    {"noui.cloud", "101.43.201.20"},   {"unique.com", "220.177.198.124"}};
int main() {
  mylist_head head;
  INIT_MY_LIST_HEAD(&head);
  DNS_entry a;
  a.domain_name = RECORDS[0][0];
  a.ip = RECORDS[0][1];
  a.type = TYPE_A;
  DNS_entry b;
  b.domain_name = RECORDS[2][0];
  b.ip = RECORDS[2][1];
  b.type = TYPE_AAAA;
  DNS_entry c;
  c.domain_name = RECORDS[4][0];
  c.ip = RECORDS[4][1];
  c.type = TYPE_A;

  mylist_add_tail(&a.node, &head);
  mylist_add_tail(&b.node, &head);
  mylist_add_tail(&c.node, &head);

  mylist_head *p;
  mylist_for_each(p, &head,DNS_entry) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s\n", entry->domain_name);
    printf("ip:%s\n", entry->ip);
    printf("type:%d\n\n",entry->type);
  }
  mylist_rotate_node_head(&b.node, &head);
  mylist_for_each(p, &head, DNS_entry) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s\n", entry->domain_name);
    printf("ip:%s\n", entry->ip);
    printf("type:%d\n\n", entry->type);
  }
  mylist_del(&c.node);
  mylist_for_each(p, &head, DNS_entry) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    printf("dn:%s\n", entry->domain_name);
    printf("ip:%s\n", entry->ip);
    printf("type:%d\n\n", entry->type);
  }
}
