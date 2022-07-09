#include "utils/map.h"
#include "console.h"

rwlock_t _map_lock;

/**
 * @brief initialize convertion map table
 *
 */
void MapInit()
{
    for(int i = 0; i < MAX_MAP_SIZE; i++)
    {
        _Map[i] = NULL;
    }
    _map_allocator = 0;
    lockInit(&_map_lock);
}

/**
 * the following consequence will cause a mistake **
 * 
 * client A -> (send id 100)  -> late (over 2 sec)
 * serial num go rounded 0 -> 65534 -> 0
 * client B -> (send id 100)  ->  serial 0
 * response of A arrive in the following 2 sec. 
 * check 0 then send back to B coz addr in Map[0 / 128] is of client B && last record is the same with last
 * 
 */

/**
 * @brief restore & convert origin id
 *
 * @param origin origin request id
 * @param addr origin request address
 * @return uint16_t converted id
 */
uint16_t addToMap(uint16_t origin, struct sockaddr_in* addr)
{
    if(addr == NULL)    return UINT16_MAX;

    /* thread-save counter */
    writeLock(&_map_lock);
    _map_allocator = (_map_allocator + 1) % UINT16_MAX;   //avoid out of bounds
    uint16_t serial = _map_allocator;
    unlock(&_map_lock);

    uint16_t converted = serial % MAX_MAP_SIZE, current = serial / MAX_MAP_SIZE;

    /* check time-out */
    if(_Map[converted] != NULL && _Map[converted]->_time_out > time(NULL))
    {
        return UINT16_MAX;
    }

    /* fill new info */
    MapNode* temp = _Map[converted];
    _Map[converted] = malloc(sizeof(MapNode));              //allocate new memory
    _Map[converted]->_time_out = time(NULL) + CONVERT_TTL;  //set new time_stamp
    _Map[converted]->_origin = origin;                      //record origin id
    _Map[converted]->_last = current;                       //record current change
    memcpy(&_Map[converted]->_from, addr, sizeof(struct sockaddr_in));  //record origin address
    free(temp);                                             //free old memory

    return serial;
}



/**
 * @brief query origin info from map table
 *
 * @param converted converted id
 * @param origin origin request id
 * @param from origin request address
 * @return int query result (0:success/-1:failed)
 */
int queryMap(uint16_t serial, uint16_t* origin, struct sockaddr_in* from)
{
    /* pre check */
    uint16_t converted = serial % MAX_MAP_SIZE, current = serial / MAX_MAP_SIZE;

    /* check if is current change */
    if(_Map[converted] == NULL || _Map[converted]->_last != current)
    {
        return -1;
        
    }

    /* copy infomation */
    memcpy(origin, &_Map[converted]->_origin, sizeof(uint16_t));
    memcpy(from, &_Map[converted]->_from, sizeof(struct sockaddr_in));

    /* clear TTL */
    MapNode* temp = _Map[converted];
    _Map[converted] = NULL;
    free(temp);

    return 0;

}


