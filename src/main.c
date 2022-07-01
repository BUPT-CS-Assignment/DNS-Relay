#include <main.h>
#include <console.h>
#include <server.h>
#pragma comment(lib,"Ws2_32.lib")

int __DEBUG__ = 0;

int main(int argc,char* argv[]){

#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2,2),&w);
#endif

    /* arg parse */
    consoleParse(argc,argv);
    
    /* server run */
    Socket DNS;
    socketInit(&DNS,SERVER_ADDR,53);

    start(&DNS);
    
#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}
