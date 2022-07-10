#ifndef __MAP_H__
#define __MAP_H__

#include "main.h"

#define MAX_MAP_SIZE        128
#define CONVERT_TTL         5

typedef struct MapNode{
    uint16_t _last;
    uint16_t _origin;
    struct sockaddr_in _from;
    time_t _time_out;

}MapNode;

static MapNode* __ID_MAP__[MAX_MAP_SIZE];
static uint16_t __MAP_ALLOC__;

void        mapInit();
uint16_t    addToMap(uint16_t origin,struct sockaddr_in* addr);
int         queryMap(uint16_t converted,uint16_t* origin,struct sockaddr_in* from);
void        checkMap();


#endif