#ifndef __HASH__
#define __HASH__
#include <stdio.h>

#define BUCKET_SIZE 1024

#define SUCCUSS  0
#define FAILURE -1

struct string_hash;
typedef struct string_hash hash;

extern void init_hash   (hash* map);
extern void count_hash  (hash* map);
extern int  insert_hash (hash* map ,const char* key ,void* value , size_t v_len);
extern int  query_hash  (hash* map ,const char* key ,void* ret   , size_t r_len);
extern int  modify_hash (hash* map ,const char* key ,void* value , size_t v_len);
#endif 
