#include <server.h>
#include <console.h>


/**
 * @brief parse from inet to formal string
 *
 * @param src origin url
 * @param dest result string dest
 * @param addtion for preference in type-mx
 * @param mode pare mode(type)
 * @param len origin url length in package
 * @param buf origin package buffer
 * @return int parse result (0:success)
 */
int urlParse(void* src, char* dest, void* addtion, int mode, uint16_t len, uint8_t* buf)
{
    if(src == NULL || dest == NULL) return SOCKET_ERROR;

    /* QNAME parse */
    if(mode == TYPE_QNAME)
    {
        return inetParse(AF_MAX, src, dest);
    }
    /* IPv4 parse */
    if(mode == TYPE_A)
    {
        return inetParse(AF_INET, src, dest);
    }
    /* IPv6 parse */
    if(mode == TYPE_AAAA)
    {
        return inetParse(AF_INET6, src, dest);
    }
    /* MX preference */
    if(mode == TYPE_MX)
    {
        *(uint16_t*)addtion = ntohs(*(uint16_t*)src);
        src = (uint8_t*)src + 2;                                        //move url pointer
        len -= 2;                                                       //change url answer length
    }
    /* CNAME/MX/NS parse */
    if(mode == TYPE_CNAME || mode == TYPE_MX || mode == TYPE_NS)
    {
        //2mxc00c   ->  mx.baidu.com  #  c00c -> 5baidu3com0 (baidu.com)
        char temp[255];                         //temp array
        memcpy(temp, src, len);                 //copy content
        char* temp_pos = temp + len - 2;        //name-pointer check position
        uint16_t name_ptr = ntohs(*(uint16_t*)((char*)src + len - 2));  //get name-pointer

        /* recursion translate */
        while((name_ptr & 0xc000) == 0xc000)    //check if is name-pointer
        {
            uint16_t offset = GET_QNAME_PTR(name_ptr);                  //count name-pointer
            char* zip_name = buf + offset;                              //get origin name string

            /* circulate copy */
            while(1)
            {
                if(*zip_name == '\0')
                {
                    name_ptr = 0;                                       //check if at the end of origin name
                    break;
                }
                uint16_t ptr_test_1 = ntohs(*(uint16_t*)zip_name);

                /* check if is name-pointer */
                if((ptr_test_1 & 0xc000) != 0xc000)                     //not name-pointer, copy by length                    
                {
                    uint8_t zip_length = *(uint8_t*)zip_name;
                    memcpy(temp_pos, zip_name, zip_length + 1);
                    temp_pos += zip_length + 1;                         //pointer move
                    zip_name += zip_length + 1;
                }
                else                                                    //is name-pointer, check the followed one
                {
                    uint16_t ptr_test_2 = ntohs(*(uint16_t*)(zip_name + 2));
                    if((ptr_test_2 & 0xc000) == 0xc000)                 //still name-pointer, continue to translate
                    {
                        memcpy(temp_pos, zip_name, sizeof(uint16_t));
                        name_ptr = ptr_test_1;
                    }
                    else                                                //not name-pointer, current translation end
                    {
                        name_ptr = 0;
                        break;
                    }
                }
            }
        }
        *temp_pos = '\0';
        return inetParse(AF_MAX, temp, dest);
    }
    return SOCKET_ERROR;

}



/**
 * @brief format url to internet form
 *
 * @param url origin url string
 * @param dest format result dest
 * @param mode format mode(type)
 * @param name origin qname string
 * @param pointer origin qname pointer
 * @param addition for preference in type-mx
 * @return int format result length
 */
int urlFormat(char* url, void* dest, int mode, char* name, uint16_t pointer, uint16_t addition)
{
    if(url == NULL || dest == NULL) return SOCKET_ERROR;

    /* IPv4 format */
    if(mode == TYPE_A)
    {
        return inetFormat(AF_INET, url, dest);
    }
    /* IPv6 format */
    if(mode == TYPE_AAAA)
    {
        return inetFormat(AF_INET6, url, dest);
    }
    /* MX preference */
    if(mode == TYPE_MX)
    {
        *(uint16_t*)dest = htons(addition);
        dest = (uint8_t*)dest + 2;          //move dest pointer
    }
    /* CNAME format */
    if(mode == TYPE_CNAME || mode == TYPE_MX || mode == TYPE_NS)
    {
        //2mxc00c   <-  mx.baidu.com  #  c00c <- 5baidu3com0 (baidu.com)
        int url_len = strlen(url), name_len = strlen(name);
        int pos = -1;
        /* compare from the end */
        for(int i = url_len - 1, j = name_len - 1; i >= 0 && j >= 0; i--, j--)
        {
            if(url[i] != name[j]) break;
            if(name[j] == '.')
            {
                pos = j + 1;            //record intact-repeat one
            }
            if(j == 0)
            {
                pos = 0;                //all repeated
            }
        }

        int res_len = inetFormat(AF_MAX, url, dest);                //pre-format
        if(pos == -1)
        {
            return res_len;             //no compress
        }
        /* compressed one layer recording to qanme */
        int cmp_len = name_len - pos;                               //compress length
        uint16_t offset = htons(pointer + pos);                     //origin pointer
        char* change_dest = dest + (res_len - 1 - cmp_len);         //replace repaeted section on pre-format
        memcpy(change_dest, &offset, sizeof(uint16_t));             //memory copy
        return res_len - cmp_len + 1 + (mode == TYPE_MX ? 2 : 0);   //return real length
    }
    return SOCKET_ERROR;

}
