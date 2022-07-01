#include <map.h>
#include <console.h>

AddrMap AddrMAP;

void MapInit(AddrMap* map)
{
    map = malloc(sizeof(AddrMap));
    map->_head = NULL;
    map->_len = 0;
    map->_lock = MAP_FREE;
}

int addToMap(AddrMap* map, uint16_t id, struct sockaddr_in* addr)
{
    /* lock check */
    if(map == NULL) return -1;
    if(map->_lock == MAP_LOCK)
    {

#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif

        if(map->_lock == MAP_LOCK)
        {
            return 1;
        }
    }
    map->_lock = MAP_LOCK;

    MapNode* mapnode = malloc(sizeof(MapNode));
    mapnode->_id = id;
    memcpy(&mapnode->_from, addr, sizeof(mapnode->_from));
    mapnode->_pre = NULL;
    mapnode->_next = NULL;

    if(map->_len != 0)
    {
        mapnode->_next = map->_head;
        map->_head->_pre = mapnode;
    }
    map->_head = mapnode;
    ++map->_len;

    map->_lock = MAP_FREE;

    return 0;
}

struct sockaddr_in*  queryMap(AddrMap* map,uint16_t id){
    /* lock check */
    if(map == NULL) return NULL;
    if(map->_lock == MAP_LOCK)
    {

#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif

        if(map->_lock == MAP_LOCK)
        {
            return NULL;
        }
    }
    map->_lock = MAP_LOCK;

    struct sockaddr_in* res = NULL;
    MapNode* temp = map->_head;
    while(temp != NULL){
        if(temp->_id == id){
            /* copy infomation */
            res = malloc(sizeof(struct sockaddr_in));
            memcpy(res,&temp->_from,sizeof(struct sockaddr_in));

            /* delete old node */
            if(temp == map->_head){
                map->_head = temp->_next;
                if(map->_head != NULL) map->_head->_pre = NULL;
            }else{
                temp->_pre->_next = temp->_next;
                if(temp->_next != NULL){
                    temp->_next->_pre  = temp->_pre;
                }
            }
            free(temp);
            --map->_len;
            break;
        }
        temp = temp->_next;
    }
    map->_lock = MAP_FREE;
    return res;

}

void mapCheck(AddrMap* map){
    consoleLog(DEBUG_L0,"> Addr Map Len: %d\n",map->_len);
    MapNode* node = map->_head;
    while(node != NULL){
        consoleLog(DEBUG_L0,"  Node: %d\n",node->_id);
        node = node->_next;
    }
}