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
                LRU_CACHE_LENGTH = atoi(argv[i] + 2);
            }
        }
        else if(argv[i][0] == '=' && len >= 2)
        {
            strcpy(_local_dns_addr, argv[i] + 1);   //local dns server address
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

