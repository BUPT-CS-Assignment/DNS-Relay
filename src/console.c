#include <main.h>
#include <console.h>
#include <stdarg.h>


/**
 * @brief parse start args
 * 
 * @param argc args number
 * @param argv args content
 */
void consoleParse(int argc, char* argv[]){
    if(argc > 1 && argv[1][0] == '-'){
        if(argv[1][1] == 'd'){
            __DEBUG__ = 1;
            if(argv[1][2] == '2'){
                __DEBUG__ = 2;
            }
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
void consoleLog(int debug_level,const char* fmt,...){
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

