#include <server.h>
#include <console.h>


/**
 * @brief url parse
 * 
 * @param src origin url pointer
 * @param dest parse result dest
 * @param mode parse mode
 * @return int parse result (0:success)
 */
int urlParse(void* src, char* dest, int mode)
{
    if(src == NULL || dest == NULL) return SOCKET_ERROR;

    /* QNAME parse */
    if(mode == TYPE_QNAME)
    {
        return inetParse(AF_MAX,src,dest);
    }
    /* IPv4 parse */
    if(mode == TYPE_A)
    {
        return inetParse(AF_INET,src,dest);
    }
    /* IPv6 parse */
    if(mode == TYPE_AAAA){
        return inetParse(AF_INET6,src,dest);
    }
    
    return SOCKET_ERROR;

}



/**
 * @brief format url to internet form
 * 
 * @param url origin url string
 * @param dest format result dest
 * @param mode format mode
 * @return int format result code (0:success)
 */
int urlFormat(char* url, void* dest, int mode)
{
    if(url == NULL || dest == NULL) return SOCKET_ERROR;
    if(mode == TYPE_A)
    {
        return inetFormat(AF_INET,url,dest);      
    }
    if(mode == TYPE_AAAA){
        return inetFormat(AF_INET6,url,dest);
    }
    return SOCKET_ERROR;
    
}
