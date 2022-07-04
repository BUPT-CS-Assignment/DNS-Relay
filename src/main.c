#include <main.h>
#include <console.h>
#include <server.h>
#pragma comment(lib,"Ws2_32.lib")

int __DEBUG__ = 0;
Socket _dns_server;
void platformInit();

int main(int argc, char* argv[])
{
    /* arg parse */
    consoleParse(argc, argv);

    /* server run */
    Socket relay_service;
    socketInit(&relay_service, DNS_RELAY_ADDR, 53);

    /* just for easy initialize. SOCKET not used*/
    socketInit(&_dns_server, LOCAL_DNS_ADDR, 53);

    start(&relay_service);

#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}

