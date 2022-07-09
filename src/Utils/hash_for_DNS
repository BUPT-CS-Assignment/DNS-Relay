#include "cache.h"
#include "utils/hash.h"

/**
 * @description: 为高层使用的支持多个的版本
 * @param {string_hash} *map
 * @param {mylist_head} *head
 * 链表头，一开始应传入空指针，函数完成后head会是动态分配的地址
 * @param {DNS_entry} *entry 要存入的条文，这里假定它已经被上层malloc过
 * @return {*}
 */
int insert_one(struct string_hash* map, mylist_head** head, DNS_entry* entry)
{
    *head = (mylist_head*)malloc(sizeof(mylist_head));
    INIT_MY_LIST_HEAD(*head);
    mylist_add_head(&entry->node, *head);
    int ret = insert_hash(map, entry->domain_name, head,
        sizeof(mylist_head*)); //只存一个指针，即head
    if(ret == SUCCUSS)
    {
        return ret;
    }
    else
    {
        mylist_head* temp;
        query_hash(map, entry->domain_name, &temp, sizeof(mylist_head*));
        //现在temp应该是取出的原链表头地址
        mylist_move_head(&entry->node, temp);
        //修改链表顺序，现在这个动态分配的entry就会被存进链表里可供查询
        //理论上来说不用改哈希桶里的值？哈希桶只是存链表dummy头节点的地址值，我把地址值取出来修改地址对应的内存内容而已，地址没有变化
        free(*head);
        *head = temp;
        return SUCCUSS;
    }
}

/**
 * @description: 从哈希图中查询某域名对应的链表头地址
 * @param {string_hash} *map
 * @param {DNS_entry} *entry 传入域名
 * @param {mylist_head} *result 地址将被保存在这里以用于调用
 * @return {*}
 */
int query_list(struct string_hash* map, DNS_entry* entry, mylist_head* result)
{
    int ret = query_hash(map, entry->domain_name, &result, sizeof(mylist_head*));
    if(ret == FAILURE)
    {
        result = NULL;
        return ret;
    }
    else
    {
        return SUCCUSS;
    }
}

/**
 * @description:
 * 查链表的测试函数，若正确则现在应该能遍历链表得到所有相同域名不同ip的条文
 * @param {string_hash} *map
 * @param {DNS_entry} *entry 要查的域名
 * @return {*}
 */
int query_one(struct string_hash* map, DNS_entry* entry)
{
    mylist_head* result;
    int ret = query_list(map, entry, result);
    if(ret == FAILURE)
    {

    return ret;
  } else {
    mylist_head *p;
    mylist_for_each(p, result) {
      DNS_entry *temp = mylist_entry(p, DNS_entry, node);
      printf("dn:%s--", entry->domain_name);
      printf("ip:%s--", entry->ip);
      printf("type:%d\n\n", entry->type);
    }
}
/**
 * @description: 从哈希桶里删除某一条文的内容
 * @param {string_hash} *map
 * @param {DNS_entry} *entry
 * @return {*}
 */
int delete_one(struct string_hash* map, DNS_entry* entry)
{
    mylist_head* head;
    int ret = query_hash(map, entry->domain_name, &head, sizeof(mylist_head*));
    if(ret = FAILURE)
    {
        return ret;
    }
    else if(mylist_is_singular(head))
    {
        //只有一个，则需要清空对应的桶内内容，这里把连着的条文一同free了，可能不对
        remove_hash(map, entry->domain_name);
        mylist_head* temp = head->next;
        DNS_entry* need_to_free = mylist_entry(temp, DNS_entry, node);
        free(need_to_free);
        free(head);
    }
    else
    {
        mylist_head* p;
        mylist_for_each(p, head)
        {
            DNS_entry* temp = mylist_entry(p, DNS_entry, node);
            // printf("dn:%s--", entry->domain_name);
            // printf("ip:%s--", entry->ip);
            // printf("type:%d\n\n", entry->type);
            if(strcmp(entry->ip, temp->ip) == 0)
            {
                mylist_head* next = p;
                p = p->prev; //
                mylist_del(next);
                free(mylist_entry(next, DNS_entry, node)); // free掉该条指定条文
                return SUCCUSS;
            }
        }
    }
}