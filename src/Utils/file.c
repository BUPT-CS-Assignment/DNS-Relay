#include "file.h"

//extern hash *map; //默认已有全局变量hash *map

/**
 * @description: 初始化所有表里的内容进入内存
 * @return {*}
 */
int file_init(hash* map)
{
    FILE* fp = fopen("host.txt", "r");
    if(fp == NULL)  return -1;
    char buffer[512];
    while(fgets(buffer, sizeof buffer, fp) && !feof(fp))
    {

        /* NOTE : In Win , the line feed signal is  '\n' ,
         *		  However in Linux , it is '\r\n'
         */

         /* remove blank line */
#ifdef _WIN32
        if(buffer[0] == '\n')
            continue;
#else
        if(buffer[0] == '\r')
            continue;
#endif

        char* val = strtok(buffer, " ");

        /* split one line string to k-v */
#ifdef _WIN32
        char* key = strtok(NULL, "\n");
#else
        char* key = strtok(NULL, "\r");
        strtok(NULL, "\n");
#endif

        // puts(key); puts(val);
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

        // if (insert_hash(map, key, val, STRMEM(val)) == FAILURE) {
        //   // memset(buffer , 0 , sizeof buffer);
        //   char concat[512];
        //   memset(concat, 0, sizeof concat);

        //   query_hash(map, key, concat, sizeof concat);

        //   // strcat(buffer , val);
        //   printf("Duplicated %s : %s + %s\n", key, concat, val);
        // }
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
int file_find(DNS_entry* entry, DNS_entry** result, hash* map)
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
            count++;
        }
        *result = malloc(sizeof(DNS_entry) * (count + 1)); //+1只是防止一些奇怪情况
        memset(*result,0,sizeof(DNS_entry) * (count + 1));
        int i = 0;
        mylist_for_each(p, res)
        {
            DNS_entry* temp = mylist_entry(p, DNS_entry, node);
            if(strcmp(temp->ip,"0.0.0.0") == 0){
                return -1;
            }
            if(temp->type == entry->type){
                memcpy(&(*result)[i], temp, sizeof(DNS_entry));
                (*result)[i].timestamp = time(NULL) + 0x80;
                i++;
            }
        }
        return i;
    }
    return 0;
}

int file_free(hash* map)
{
    free_hash(map); // free_node() modified
}