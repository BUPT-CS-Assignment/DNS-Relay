#include "main.h"
#include "utils/list.h"
#include "utils/hash.h"

typedef struct
{
	char type;
	char ip[16];
    mylist_head node;

} payload;

void init_hash_list(hash* map) {
	init_hash(map);
}

void insert_hash_list(
	hash* map , 
	const char* key , 
	payload* _new) 
{
	// 如果key已经存在，则插入到对应链表的尾部
	// 注意给定的_new是一个指针，表示将该指针对应的内存空间加入到哈希表中
	if(insert_hash(map , key , _new , sizeof(*_new)) == FAILURE) {
		payload target;

		// 返回临时的查询结果：链表头部
		query_hash(map , key , &target , sizeof target);

		// 将链表新节点的内容保存一份
		payload* nspace = calloc(1 , sizeof(payload));
		memcpy(nspace , _new , sizeof(payload));

		// 将新节点插入到尾部
		mylist_add_tail(&nspace->node , &target.node);
	}
}

// 返回链表的头部
// 如果要删除链表中的某一结点，记得free()
int query_hash_list(hash* map , const char* key , payload* ret) 
{
	return query_hash(map , key , ret , sizeof(payload));
}