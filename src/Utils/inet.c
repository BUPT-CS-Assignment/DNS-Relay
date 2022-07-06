#include <main.h>
#include <console.h>


/**
 * @brief parse url sourse to string
 *
 * @param af inet family
 * @param url_src url inet sourse
 * @param dest url string dest pointer
 * @return int parse result (0:success)
 */
int inetParse(int af, void* url_src, char* dest)
{
    int ret;

    /* --------------------------------- IPv4 from Net to String ---------------------------------*/
    if(af == AF_INET)
    {
#ifdef _WIN32
        struct sockaddr_in temp;                //set sockaddr struct
        int len = 16;                           //set length   
        temp.sin_family = af;                   //set address family
        memcpy(&temp.sin_addr.s_addr, url_src, sizeof(struct in_addr));   //copy to struct
        ret = WSAAddressToString((LPSOCKADDR)&temp, sizeof(struct sockaddr_in), 0, dest, (LPDWORD)&len);
#else
        char* res = inet_ntop(AF_INET, url_src, dest, 16);
        if(res == NULL) ret = SOCKET_ERROR;
#endif
        return ret;

}

    /* --------------------------------- IPv6 from Net to String ---------------------------------*/
    if(af == AF_INET6)
    {
#ifdef _WIN32
        struct sockaddr_in6 temp;               //set sockaddr6 struct
        int len = 40;                           //set length
        temp.sin6_family = af;                  //set address family
        memcpy(&temp.sin6_addr.u, url_src, sizeof(struct in6_addr));      //copy to struct
        ret = WSAAddressToString((LPSOCKADDR)&temp, sizeof(struct sockaddr_in6), 0, dest, (LPDWORD)&len);
#else
        char* res = inet_ntop(AF_INET6, url_src, dest, 40);
        if(res == NULL) ret = SOCKET_ERROR;
#endif
        return ret;

    }

    /* --------------------------------- Dname from Net to String ---------------------------------*/
    if(af == AF_MAX)
    {
        int pos = 0;
        while(((char*)url_src)[pos] != '\0')
        {
            int len = ((char*)url_src)[pos];    //length byte
            /* read chars */
            len = (len > 64 ? 64 : len);        //max length 64 bytes
            for(int i = 0; i < len; i++)
            {
                dest[pos + i] = ((char*)url_src)[pos + i + 1];
            }
            dest[pos + len] = '.';              //add '.'
            pos += (len + 1);                   //pointer move
        }
        dest[pos - 1] = '\0';
    }
    return SOCKET_ERROR;
}



/**
 * @brief format url into inet form
 *
 * @param af inet family
 * @param url_src url string source
 * @param dest url inet dest pointer
 * @return int format length
 */
int inetFormat(int af, char* url_src, void* dest)
{
    int ret;

    /* --------------------------------- IPv4 from String to Net ---------------------------------*/
    if(af == AF_INET)
    {
#ifdef _WIN32
        struct sockaddr_in temp;
        int len = sizeof(struct sockaddr_in);
        ret = WSAStringToAddress(url_src, AF_INET, NULL, (LPSOCKADDR)&temp, (LPINT)&len);
        memcpy(dest, &temp.sin_addr, 4);
        if(ret != 0)    return 0;
#else
        ret = inet_pton(AF_INET, url_src, dest);
        if(ret != 1)    return 0;
#endif
        return 4;

    }

    /* --------------------------------- IPv6 from String to Net ---------------------------------*/
    if(af == AF_INET6)
    {

#ifdef _WIN32
        struct sockaddr_in6 temp;
        int len = sizeof(struct sockaddr_in6);
        ret = WSAStringToAddress(url_src, AF_INET6, NULL, (LPSOCKADDR)&temp, (LPINT)&len);
        memcpy(dest, &temp.sin6_addr, 16);
        if(ret != 0)    return 0;
#else
        ret = inet_pton(AF_INET6, url_src, dest);
        if(ret != 1)    return 0;
#endif
        return 16;

}

    /* --------------------------------- Dname from String to Net ---------------------------------*/
    //"www.test.com.cn" -> "3www4test3com2cn"
    if(af == AF_MAX)
    {
        char* src_pos = url_src;                        //origin url pointer
        uint8_t* len_pos = dest;                        //legnth_prefix pointer
        uint8_t* dest_pos = dest + 1;                   //dest pointer
        while(*src_pos != '\0')
        {                        //not the end of string
            uint8_t len = 0;
            while(*src_pos != '.' && *src_pos != '\0')  //not divider or end
            { 
                len ++;                                 //record part length
                *dest_pos++ = *src_pos++;               //read & copy next
            }
            *len_pos = len;                             //set part length
            len_pos = dest_pos;                         //move length_prefix pointer
            ++ dest_pos; ++src_pos;                     //pointer move on
        }
        *(dest_pos - 1) = '\0';                           //set end (not necessary)
        return strlen(url_src) + 1;                     //return total len of format result
    }
    return 0;
}