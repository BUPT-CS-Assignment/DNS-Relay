#include "host.h"

/**
 * @description: 初始化所有表里的内容进入内存
 * @return {*}
 */
int hostInit(hash* map)
{
    FILE* fp = fopen(__HOST_DEST__, "r");
    if(fp == NULL)  return -1;
    char buffer[512];
    while(fgets(buffer, sizeof buffer, fp) && !feof(fp))
    {

        if(buffer[0] == '\n')   continue;

        char* val = strtok(buffer, " ");

        /* split one line string to k-v */
        char* key = strtok(NULL, "\n");

        /*现在假定val是某域名对应的ip*/
        DNS_entry* entry = (DNS_entry*)malloc(sizeof(DNS_entry));
        entry->domain_name = malloc(STRMEM(key));
        entry->ip = malloc(STRMEM(val));
        memcpy(entry->domain_name, key, STRMEM(key));
        memcpy(entry->ip, val, STRMEM(val));

        entry->type = TYPE_A; //一定吗？
        entry->timestamp = 0;
        entry->addition = 0;

        mylist_head* head;

        int ret = query_hash(map, entry->domain_name, &head, sizeof(mylist_head*));
        if(ret == SUCCUSS)
        {
            //该位置已经找到了kv对，说明已经有一个链表头，不用再初始化新的，而是把新节点加进链表里
            mylist_add_head(&entry->node, head);

        }
        else
        {
            //没有找到，对应位置加入一个新头
            head = malloc(sizeof(mylist_head));
            INIT_MY_LIST_HEAD(head);
            mylist_add_head(&entry->node, head);
            insert_hash(map, entry->domain_name, &head, sizeof(mylist_head*));
        }
    }
    count_hash(map);
    fclose(fp);
    return 0;
}



/**
 * @description: 提供给上层的查找函数
 * @param {DNS_entry} *entry 要查找的条文类型
 * @param {DNS_entry} **result 查找结果数组的地址，使用前应该只需要声明，查到的结果将会全部保存在此处，是临时动态分配的指针，需要随用随free（不需要深拷贝）
 * @return count 返回查找到条文的数目
 */
int hostQuery(DNS_entry* entry, DNS_entry** result, hash* map, int* flag)
{
    mylist_head* res;
    int ret = query_hash(map, entry->domain_name, &res, sizeof(mylist_head*));
    if(ret == FAILURE)
    {
        return 0;
    }
    else
    {
        mylist_head* p;
        int count = 0;
        mylist_for_each(p, res)
        {
            if(flag == 0)   return 0;
            count++;
        }
        *result = malloc(sizeof(DNS_entry) * count);
        memset(*result, 0, sizeof(DNS_entry) * count);

        int num = 0;
        mylist_for_each(p, res)
        {
            if(flag == 0)   return 0;

            DNS_entry* temp = mylist_entry(p, DNS_entry, node);
            if(strcmp(temp->ip, "0.0.0.0") == 0)
            {
                return -1;
            }
            if(temp->type == entry->type)
            {
                memcpy(&(*result)[num], temp, sizeof(DNS_entry));
                (*result)[num].timestamp = time(NULL) + 0x80;
                num++;
            }
        }
        return num;
    }
    return 0;
}


int hostFree(hash* map)
{
    free_hash(map); // free_node() modified
}