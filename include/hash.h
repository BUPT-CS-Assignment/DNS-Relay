#ifndef __HASH__
#define __HASH__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define BUCKET_SIZE 1024
#define SUCCUSS  0
#define FAILURE -1
#define STRMEM(s) (strlen(s) + 1)

/**
 * @brief 哈希桶中的节点，记录K-V对的值
 * 
 * @param key 	存储键的内存地址
 * @param value 存储值的内存地址
 * 
 */
typedef struct hash_node 
{
	void* key;
	void* value;
	
	size_t key_s;
	size_t val_s;
	
	struct hash_node *next;

} hash_node;


/**
 * @brief 线程安全的哈希桶，维护节点链表
 * 
 */
typedef struct 
{
	pthread_rwlock_t rwlock;
	
	hash_node *head , *tail;
	
	size_t bk_s;
	
} hash_bucket;


/**
 * @brief 字符串哈希表的偏特化
 * 
 */
typedef struct string_hash 
{
	hash_bucket bks[BUCKET_SIZE];

}hash;


/**
 * @brief 初始化哈希表
 * 
 * @param map 待初始化哈希表的地址
 */
extern void init_hash   (hash* map);

/**
 * @brief 释放哈希表
 * 
 * @param map 待释放哈希表的地址
 */
extern void free_hash   (hash* map);
/**
 * @brief 哈希表的辅助统计函数，输出桶的平均深度的最大深度
 * 
 * @param map 待统计的哈希表地址
 */
extern void count_hash  (hash* map);

/**
 * @brief 向哈希表中插入一个K-V对
 * 
 * @param map 目标哈希表
 * @param key 起键作用的字符串
 * @param value 指向待存储的信息的内存地址
 * @param v_len 待存储信息的BYTE长度
 * @return int 插入操作的状态：*SUCCESS*表示成功；*FAILURE*表示存在键的重复
 * 
 */
extern int  insert_hash (hash* map ,const char* key ,void* value , size_t v_len);


/**
 * @brief 哈希表中删除一个K-V对
 * 
 * @param map 目标哈希表
 * @param key 起键作用的字符串
 * @return int 插入操作的状态：*SUCCESS*表示成功；*FAILURE*表示不存在键
 * 
 */
extern int  remove_hash  (hash* map ,const char* key);

/**
 * @brief 向哈希表中查询一个K-V对
 * 
 * @param map 目标哈希表
 * @param key 起键作用的字符串
 * @param ret 返回查询信息的缓冲区地址
 * @param r_len 缓冲区的大小
 * @return int 查询操作的状态：*SUCCESS*表示成功；*FAILURE*表示不存在键
 * 
 */
extern int  query_hash  (hash* map ,const char* key ,void* ret   , size_t r_len);

/**
 * @brief 修改哈希表中一个K-V对
 * 
 * @param map 目标哈希表
 * @param key 起键作用的字符串
 * @param value 新的待存储信息
 * @param v_len 待存储信息的BYTE长度
 * @return int 修改操作的状态：*SUCCESS*表示成功；*FAILURE*表示不存在该键
 * 
 */
extern int  modify_hash (hash* map ,const char* key ,void* value , size_t v_len);

#endif 
