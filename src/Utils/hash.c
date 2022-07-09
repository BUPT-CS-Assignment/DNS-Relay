#include "utils/hash.h"

/* -----------------GENERIC Hash Node Implement----------------- */
/** 
 * @brief use void* to generalize
 */
static hash_node* make_node(
	const void* _key , size_t _k_len ,
	void* _value 	 , size_t _v_len ) 
{	
	hash_node* node = (hash_node*) calloc(1 , sizeof(hash_node));

	node->next = NULL;

	node->key   = calloc(1 , _k_len);
	node->value = calloc(1 , _v_len);

	node->key_s = _k_len;
	node->val_s = _v_len;

	memcpy(node->key   , _key 	, _k_len);
	memcpy(node->value , _value , _v_len);
	
	return node;
}

static void modify_node(hash_node* node , void* _value , size_t _v_len) 
{
	node->value = realloc(node->value , _v_len);
	node->val_s = _v_len;

	memcpy(node->value , _value , _v_len);
}

static void free_node(hash_node* node) 
{
	free(node->key	);
	free(node->value);
	free(node);
}

static void cat_node(hash_node* prev ,hash_node* nxtv ) 
{
	prev->next = nxtv;
}

#define KEY_MATCH(node , k , k_s) \
		(node->key_s == k_s && !memcmp(node->key , k , node->key_s))

/* -----------------GENERIC Hash Bucket Implement----------------- */
/** 
 * @brief  It based on single link and Read-Write lock to 
 * 		   guarantee Thead-Safety
 */
static void make_bucket(hash_bucket* bk) 
{
	pthread_rwlock_init(&bk->rwlock , NULL);
	
	bk->head = bk->tail = NULL;
	bk->bk_s = 0;
}

static void free_bucket(hash_bucket* bk) 
{
	pthread_rwlock_destroy(&bk->rwlock);

	for(hash_node *now = bk->head , *prev = NULL; 
				   now ; now = now->next) 
		{
			if (prev != NULL) 
				free_node(prev);

			prev = now;
		}
}

static int insert_bucket(hash_bucket* bk , hash_node* node) 
{
	pthread_rwlock_wrlock(&bk->rwlock);

	if (bk->head == NULL) {
		bk->head = bk->tail = node;
		bk->bk_s = 1;
		
		pthread_rwlock_unlock(&bk->rwlock);
		return SUCCUSS;
	}

	for(hash_node *now = bk->head ; now ; now = now->next) 
		if (KEY_MATCH(now , node->key , node->key_s)){

			/* IF there is a matched key */
			pthread_rwlock_unlock(&bk->rwlock);
			return FAILURE;
		}
	
	cat_node(bk->tail , node);
	bk->tail = node;
	bk->bk_s++;

	pthread_rwlock_unlock(&bk->rwlock);
	return SUCCUSS;
}


static int remove_bucket(
	hash_bucket* bk , 
	const void* key , size_t k_len) 
{
	pthread_rwlock_wrlock(&bk->rwlock);

	if (KEY_MATCH(bk->head , key , k_len) ){
		
		hash_node* prev = bk->head;
		bk->head = bk->head->next;
		free_node(prev);
		
		if(!(--bk->bk_s)) 
		   bk->tail = NULL;


		pthread_rwlock_unlock(&bk->rwlock);
		return SUCCUSS;
	}

	for(hash_node *now = bk->head , *prev = NULL; 
				   now ; now = now->next) {

		if (KEY_MATCH(now , key , k_len)){
			
			cat_node(prev , now->next);
			bk->bk_s--;
			
			if (bk->tail == now)
				bk->tail = prev;

			pthread_rwlock_unlock(&bk->rwlock);
			return SUCCUSS;
		}
		prev = now;
	}

	pthread_rwlock_unlock(&bk->rwlock);
	return FAILURE;
}


static int query_bucket(
	hash_bucket* bk , 
	hash_node*  ret , 
	const void* key , size_t k_len) 
{
	pthread_rwlock_rdlock(&bk->rwlock);

	for(hash_node *now = bk->head ; now ; now = now->next) 
		if (KEY_MATCH(now , key , k_len)){
		
			memcpy(ret , now , sizeof(hash_node));
			pthread_rwlock_unlock(&bk->rwlock);
			return SUCCUSS;
		}

	pthread_rwlock_unlock(&bk->rwlock);
	return FAILURE;
}

static int modify_bucket(
	hash_bucket* bk , 
	const void* key , size_t k_len ,
	void* value 	, size_t v_len )
{
	pthread_rwlock_wrlock(&bk->rwlock);

	for(hash_node *now = bk->head ; now ; now = now->next) 
		if (KEY_MATCH(now , key , k_len)){

			modify_node(now , value , v_len);
			pthread_rwlock_unlock(&bk->rwlock);
			return SUCCUSS;
		}
	
	pthread_rwlock_unlock(&bk->rwlock);
	return FAILURE;
}


/* -----------------FNV32-1a Algorithm Implement----------------- */
/**
 * @brief Click (https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function)
 * 		  for more infomathon
 */
#define FNV_PRIME 	16777619
#define OFFET_BASIS 2166136261

static uint32_t fnv32(const char* str , size_t len) {
	uint32_t hash = OFFET_BASIS;

	for(size_t i = 0 ; i < len ; i++) {
		hash ^= (uint8_t)str[i];
		hash *= FNV_PRIME;
	}

	return hash;
}

/* -----------------String Hash Map Implement----------------- */
/** 
 * @brief use FNV-32 algorithm to calculate string hash value.
 * 		  have a fixed bucket size
 * 		  enable to keep a copy of key and value 
 * @param key 	const char[] TYPE
 * @param value void* TYPE
 *
 */

const size_t HASH_MASK = BUCKET_SIZE - 1;

void init_hash(struct string_hash* map) 
{
	for(size_t i = 0 ; i < BUCKET_SIZE ; i++) 
		make_bucket(&map->bks[i]);
}

void free_hash(struct string_hash* map)
{
	for(size_t i = 0 ; i < BUCKET_SIZE ; i++) 
		free_bucket(&map->bks[i]);	
}

int insert_hash(
	struct string_hash* map ,
	const char* key ,
	void* value , size_t v_len)
{
	size_t k_len = STRMEM(key);
	size_t bk_id = fnv32(key , k_len) & HASH_MASK;

	return insert_bucket(&map->bks[bk_id] , make_node(
		key   , k_len ,
		value , v_len
	));

}

int remove_hash(
	struct string_hash* map ,
	const char* key)
{
	size_t k_len = STRMEM(key);
	size_t bk_id = fnv32(key , k_len) & HASH_MASK;
	
	return remove_bucket(&map->bks[bk_id] , key , k_len);
}

int query_hash(
	struct string_hash* map ,
	const char* key ,
	void* ret , size_t r_len)
{
	size_t k_len = STRMEM(key);
	size_t bk_id = fnv32(key , k_len) & HASH_MASK;

	hash_node __ret;

	int flag = query_bucket(&map->bks[bk_id] , &__ret , key , k_len);
	
	if (flag == SUCCUSS)
		memcpy(ret ,  __ret.value , __ret.val_s);

	return flag;
}

int modify_hash(
	struct string_hash* map ,
	const char* key ,
	void* value , size_t v_len)
{
	size_t k_len = STRMEM(key);
	size_t bk_id = fnv32(key , k_len) & HASH_MASK;

	return modify_bucket(&map->bks[bk_id],
		key   , k_len ,
		value , v_len		
	);
}

void count_hash(struct string_hash* map)
{
	size_t tmp = 0;
	size_t sum = 0;
	
	for(int i = 0 ; i < BUCKET_SIZE ; i++)
		tmp = tmp > (map->bks[i].bk_s) ? tmp : map->bks[i].bk_s ,
		sum += map->bks[i].bk_s;
	printf("Max Bucket Nodes = %lu\n"
		   "Average deepth   = %lf\n",
		   tmp , (double)sum / BUCKET_SIZE);
	
}

