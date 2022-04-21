#include <server.h>

/* url parse */
int UrlParse(void *src, char *dest, int mode){
    if(dest == NULL) return 0;

    /* QNAME parse */
    if(mode == TYPE_QNAME){
        int pos = 0;
        while(((char *)src)[pos] != '\0'){
            /* length byte */
            int len = ((char *)src)[pos];

            /* read chars */
            len = (len > 64 ? 64 : len);
            for(int i = 0; i < len; i++){
                dest[pos + i] = ((char *)src)[pos + i + 1];
            }
            dest[pos + len] = '.';   //add '.'
            pos += (len + 1);   //pointer move
        }
        dest[pos - 1] = '\0';


    }
    /* IPv4 parse */
    else if(mode == TYPE_A){
        dest[3] = dest[7] = dest[11] = '.'; dest[15] = '\0';
        char *temp = (char *)malloc(4);

        /* uint32_t to char* */
        for(int i = 0; i < 4; i++){
            itoa(*(uint8_t *)src, temp, 10);
            memcpy(dest + i * 4, temp, 3);
            src++;
        }
        /* memory free */
        free(temp);

    }
    else{
        return 0;
    }
    return 1;

}

/* url format */
int UrlFormat(char *url, void *dest, int mode){
    if(dest == NULL) return 0;
    /* IPv4 format */
    if(mode == TYPE_A){
        printf("url : %s\n", url);
        char temp[strlen(url) + 1];
        strcpy(temp, url);
        char *ptr = strtok(temp, ".");
        for(int i = 0; i < 4; i++){
            *(uint32_t *)dest |= (atoi(ptr) << ((3 - i) * 8));
            ptr = strtok(NULL, ".");
        }
        return 1;
    }
    else{
        return NULL;
    }

}

