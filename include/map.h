#ifndef __MAP_H__
#define __MAP_H__

#include "main.h"

#define DEFAULT_MAP_LEN     128
#define MAP_LEN_ADD         64

#define MAP_FREE            0
#define MAP_LOCK            1
typedef struct MapNode
{
    uint16_t _id;
    struct sockaddr_in _from;
    struct MapNode* _pre;
    struct MapNode* _next;
    
}MapNode;

typedef struct AddrMap
{
    MapNode* _head;
    size_t _len;
    uint8_t _lock;

}AddrMap;

extern AddrMap AddrMAP;

/* Map Query */
void MapInit(AddrMap* map);
int addToMap(AddrMap* map, uint16_t id, struct sockaddr_in* addr);
struct sockaddr_in*  queryMap(AddrMap* map,uint16_t id);
void mapCheck(AddrMap* map);

#endif