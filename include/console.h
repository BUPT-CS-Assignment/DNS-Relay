#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define DEBUG_L0 0
#define DEBUG_L1 1
#define DEBUG_L2 2

#define NONE -1
#define ERROR 0
#define SUCCESS 1
#define WARNING 2


extern int __DEBUG__;
void ConsoleParse(int argc,char* argv[]);
int ConsoleLog(int,int,const char *);

#endif