#include <map.h>
#include <console.h>


/**
 * @brief initialize convertion map table
 *
 */
void MapInit()
{
    memset(_Map, 0, sizeof(_Map));
    _map_allocator = 0;
}



/**
 * @brief restore & convert origin id
 *
 * @param origin origin request id
 * @param addr origin request address
 * @return uint16_t converted id
 */
uint16_t addToMap(uint16_t origin, struct sockaddr_in* addr)
{
    if(addr == NULL)    return;

    /* add allocator counter*/
    _map_allocator = (_map_allocator + 1) % MAX_MAP_SIZE;
    uint16_t converted = _map_allocator;

    /* wait for old-timeout */
    while(_Map[converted]._time_out > time(NULL));

    /* fill new info */
    _Map[converted]._time_out = time(NULL) + MAP_TTL;
    _Map[converted]._origin = origin;
    memcpy(&_Map[converted]._from, addr, sizeof(struct sockaddr_in));

    return converted;

}



/**
 * @brief query origin info from map table
 * 
 * @param converted converted id
 * @param origin origin request id
 * @param from origin request address
 * @return int query result (0:success/1:failed)
 */
int queryMap(uint16_t converted, uint16_t* origin, struct sockaddr_in* from)
{
    if(origin == NULL || from == NULL || converted >= MAX_MAP_SIZE)
    {
        return 1;
    }

    /* copy infomation */
    memcpy(origin,&_Map[converted]._origin,sizeof(uint16_t));
    memcpy(from,&_Map[converted]._from,sizeof(struct sockaddr_in));

    /* clear TTL */
    _Map[converted]._time_out = 0;

    return 0;

}


