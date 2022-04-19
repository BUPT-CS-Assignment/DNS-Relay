#include <main.h>
#include <console.h>

int __DEBUG__ = 0;

void ConsoleParse(int argc, char* argv[]){
    if(argc > 1 && argv[1][0] == '-'){
        if(argv[1][1] == 'd'){
            __DEBUG__ = 1;
            if(argv[1][2] == '2'){
                __DEBUG__ = 2;
            }
        }
    }
}

int ConsoleLog(int return_val,int debug_val,const char* msg){
    if(debug_val <= __DEBUG__){
        printf("%s\n",msg);
    }
    return return_val;
}