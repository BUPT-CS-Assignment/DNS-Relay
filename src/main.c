#include <main.h>
#include <console.h>
#include <server.h>
#pragma comment(lib,"Ws2_32.lib")

int main(int argc,char* argv[]){

    /* lib enable */
    WSADATA w;
    WSAStartup(MAKEWORD(2,2),&w);

    /* arg parse */
    ConsoleParse(argc,argv);
    
    /* server run */
    Server DNS;
    if(!ServerInit(&DNS)) return -1;
    Start(&DNS);
    
    system("pause");
    return 0;
}
