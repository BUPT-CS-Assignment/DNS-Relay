#include "utils/map.h"
#include "console.h"


/**
 * @brief initialize convertion map table
 *
 */
void MapInit()
{
    for(int i = 0; i < MAX_MAP_SIZE; i++){
        _Map[i] = NULL;
    }
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
    if(addr == NULL)    return UINT16_MAX;

    /* add allocator counter*/
    _map_allocator = (_map_allocator + 1) % MAX_MAP_SIZE;   //avoid out of bounds
    uint16_t converted = _map_allocator;                    //allocate converted id

    /* wait for old-timeout */
    while(_Map[converted] != NULL && _Map[converted]->_time_out > time(NULL));

    /* fill new info */
    MapNode* temp = _Map[converted];
    _Map[converted] = malloc(sizeof(MapNode));              //allocate new memory
    free(temp);                                             //free old memory
    _Map[converted]->_time_out = time(NULL) + CONVERT_TTL;  //set new time_stamp
    _Map[converted]->_origin = origin;                      //record origin id
    memcpy(&_Map[converted]->_from, addr, sizeof(struct sockaddr_in));  //record origin address

    return converted;

}



/**
 * @brief query origin info from map table
 * 
 * @param converted converted id
 * @param origin origin request id
 * @param from origin request address
 * @return int query result (0:success/-1:failed)
 */
int queryMap(uint16_t converted, uint16_t* origin, struct sockaddr_in* from)
{
    /* pre check */
    if(origin == NULL || from == NULL || converted >= MAX_MAP_SIZE || _Map[converted] == NULL)
    {
        return -1;
    }

    /* copy infomation */
    memcpy(origin,&_Map[converted]->_origin,sizeof(uint16_t));
    memcpy(from,&_Map[converted]->_from,sizeof(struct sockaddr_in));

    /* clear TTL */
    MapNode* temp = _Map[converted];
    _Map[converted] = NULL;


    return 0;

}


