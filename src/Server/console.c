#include "server.h"
#include "console.h"
#include "stdarg.h"


/**
 * @brief parse start args
 *
 * @param argc args number
 * @param argv args content
 */
void consoleParse(int argc, char* argv[])
{
    for(int i = 1; i < argc; i++)
    {
        int len = strlen(argv[i]);
        if(argv[i][0] == '-' && len >= 2)
        {
            if(argv[i][1] == 'd')       // debug level
            {
                __DEBUG__ = 1;
                if(len >= 3 && argv[i][2] == '2')
                {
                    __DEBUG__ = 2;      //debug level 2
                }
            }
            else if(argv[i][1] == 'c' && len >= 3)  //cache size
            {
                __CACHE_LEN__ = atoi(argv[i] + 2);
            }
        }
        else if(argv[i][0] == '=' && len >= 2)
        {
            strcpy(__LOCAL_DNS_ADDR__, argv[i] + 1);   //local dns server address
        }
        else if(argv[i][0] == ':' && len >= 2)
        {
            strcpy(__HOST_DEST__, argv[i] + 1);
        }
    }

}



/**
 * @brief log infomation on console
 *
 * @param debug_level log upon debug level
 * @param fmt printf() format string
 * @param ... printf() var_args
 */
void consoleLog(int debug_level, const char* fmt, ...)
{
    /* DEBUG level check */
    if(debug_level > __DEBUG__) return;

    /* (printf) var_arg */
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    /* color reset */
    printf(RESET);
}



/**
 * @brief debug operation input while running
 *
 * @return void*
 */
void* debugHandle()
{
    char cmd[255];
    while(1)
    {
        printf(BOLDWHITE"> ");
        scanf("%[^\n]", (char*)&cmd);
        if(cmd[0] == 'd')
        {
            if(cmd[1] == '2') __DEBUG__ = DEBUG_L2;
            else if(cmd[1] == '1') __DEBUG__ = DEBUG_L1;
            else __DEBUG__ = DEBUG_L0;
            consoleLog(DEBUG_L0, BOLDCYAN"> debug level reset: L%d\n", __DEBUG__);
        }
        else if(cmd[0] == 't')
        {
            consoleLog(DEBUG_L0, BOLDRED"> thread num: %d\n", __THREAD__);
        }
        else if(strcmp(cmd, "cache-c") == 0)
        {
            cacheCheck(__URL_CACHE__);
        }
        else if(strcmp(cmd, "flush") == 0)
        {
            cacheFlush(__URL_CACHE__);
        }
        else if(strcmp(cmd, "cache-o") == 0)
        {
            cacheOutput(__URL_CACHE__);
        }
        else if(strncmp(cmd, "cache-s", 7) == 0 && strlen(cmd) >= 6)
        {
            int time = atoi((char*)cmd + 8);
            __CACHE_SCAN_TIME__ = (time > 30 ? time : 30);
            consoleLog(DEBUG_L0, BOLDRED"> cache scan period: %ds\n", __CACHE_SCAN_TIME__);
        }
        else if(strncmp(cmd, "server", 6) == 0)
        {
            uint32_t res = inet_addr((char*)cmd + 7);
            if(res != INADDR_NONE)
            {
                __DNS_SERVER__._addr.sin_addr.s_addr = res;
                consoleLog(DEBUG_L0, BOLDCYAN"> local dns server reset: %s\n", (char*)cmd + 7);
            }
        }
        else
        {
            consoleLog(DEBUG_L0, BOLDRED"> command undefined.\n");
        }
        setbuf(stdin, NULL);
    }
}

