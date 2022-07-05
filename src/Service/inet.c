#include <main.h>
#include <console.h>
#include <server.h>


/**
 * @brief parse url sourse to string
 * 
 * @param af inet family
 * @param url_src url inet sourse
 * @param dest url string dest pointer
 * @return int parse result (0:success)
 */
int inetParse(int af,void* url_src,char* dest){
    int ret;
    if(af == AF_INET){
        struct sockaddr_in temp;                //set sockaddr struct
        int len = 16;                           //set length   
        temp.sin_family = af;                   //set address family
        memcpy(&temp.sin_addr.s_addr,url_src,sizeof(struct in_addr));   //copy to struct
        ret = WSAAddressToString((LPSOCKADDR)&temp,sizeof(struct sockaddr_in),0,dest,&len);
        return ret;
    }
    if(af == AF_INET6){
        struct sockaddr_in6 temp;               //set sockaddr6 struct
        int len = 40;                           //set length
        temp.sin6_family = af;                  //set address family
        memcpy(&temp.sin6_addr.u,url_src,sizeof(struct in6_addr));      //copy to struct
        ret = WSAAddressToString((LPSOCKADDR)&temp,sizeof(struct sockaddr_in6),0,dest,&len);
        return ret;
    }
    if(af == AF_MAX){
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
 * @return int format result (0:success)
 */
int inetFormat(int af,char* url_src,void* dest){
    int ret;
    if(af == AF_INET){
        struct sockaddr_in temp;
        int len = sizeof(struct sockaddr_in);

#ifdef _WIN32
        ret = WSAStringToAddress(url_src,AF_INET,NULL,(LPSOCKADDR)&temp,&len);
        if(ret != 0)    return ret;
#else

#endif

        memcpy(dest,&temp.sin_addr.S_un.S_addr,4);
        return 0;
    }
    if(af == AF_INET6){
        struct sockaddr_in6 temp;
        int len = sizeof(struct sockaddr_in6);

#ifdef _WIN32
        ret = WSAStringToAddress(url_src,AF_INET6,NULL,(LPSOCKADDR)&temp,&len);
        if(ret != 0)    return ret;
#else

#endif

        memcpy(dest,&temp.sin6_addr.u.Byte,16);
        return 0;
    }
    //"abc.dd.e.f.ts" -> "3abc2dd1e1f2ts"
    if(af == AF_MAX){
        char* src_pos = url_src;                //origin url pointer
        uint8_t* len_pos = dest;                //legnth_prefix pointer
        uint8_t* dest_pos = dest + 1;           //dest pointer
        while(*src_pos != '\0'){                //not the end of string
            uint8_t len = 0;
            while(*src_pos != '.' && *src_pos != '\0'){     //not divider or end
                len ++;                         //record part length
                *dest_pos++ = *src_pos++;       //read & copy next
            }
            *len_pos = len;                     //set part length
            len_pos = dest_pos;                 //move length_prefix pointer
            ++ dest_pos; ++src_pos;             //pointer move on
        }
        *(dest_pos-1) = '\0';                   //set end (not necessary)
        return strlen(url_src) + 1;             //return total len of format result
    }
    return SOCKET_ERROR;
}