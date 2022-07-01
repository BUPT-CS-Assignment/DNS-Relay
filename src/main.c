#include <main.h>
#include <console.h>
#include <server.h>
#pragma comment(lib,"Ws2_32.lib")

int __DEBUG__ = 0;

int main(int argc,char* argv[]){

    WSADATA w;
    WSAStartup(MAKEWORD(2,2),&w);

    /* arg parse */
    consoleParse(argc,argv);
    
    /* server run */
    Socket DNS;
    socketInit(&DNS,SERVER_ADDR,53);

    start(&DNS);
    
    WSACleanup();
    system("pause");
    return 0;
}
