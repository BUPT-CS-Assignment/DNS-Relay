#include <server.h>

/**
 * @brief Url Parse
 *
 * @param void* src
 * @param char* dest
 * @param int mode
 * @return int
 */
int urlParse(void* src, char* dest, int mode)
{
    if(dest == NULL) return 0;

    /* QNAME parse */
    if(mode == TYPE_QNAME)
    {
        int pos = 0;
        while(((char*)src)[pos] != '\0')
        {
            /* length byte */
            int len = ((char*)src)[pos];

            /* read chars */
            len = (len > 64 ? 64 : len);  //max length 64 bytes
            for(int i = 0; i < len; i++)
            {
                dest[pos + i] = ((char*)src)[pos + i + 1];
            }
            dest[pos + len] = '.';  //add '.'
            pos += (len + 1);       //pointer move
        }
        dest[pos - 1] = '\0';
    }
    /* IPv4 parse */
    else if(mode == TYPE_A)
    {

        char* temp = (char*)malloc(4);
        char* pos = dest;
        int len = 0;

        /* uint32_t to char* */
        for(int i = 0; i < 4; i++)
        {

#ifdef _WIN32
            itoa(*(uint8_t*)src, temp, 10);
#else
            sprintf(temp, "%d", *(uint8_t*)src);
#endif

            int len = strlen(temp);
            strcpy(pos, temp);
            *(pos + len) = '.';
            ++ src;
            pos += (len + 1);
        }
        *(pos - 1) = '\0';
        /* memory free */
        free(temp);

    }
    else
    {
        return 0;
    }
    return 1;
}


/**
 * @brief Format Url to Chars
 *
 * @param char* url
 * @param void* dest
 * @param int mode
 * @return int
 */
int urlFormat(char* url, void* dest, int mode)
{
    if(dest == NULL) return 0;
    /* IPv4 format */
    if(mode == TYPE_A)
    {
        char temp[strlen(url) + 1];
        strcpy(temp, url);
        char* ptr = strtok(temp, ".");
        for(int i = 0; i < 4; i++)
        {
            *(uint32_t*)dest |= (atoi(ptr) << ((3 - i) * 8));
            ptr = strtok(NULL, ".");
        }
        return 1;
    }
    else
    {
        return 0;
    }
}



