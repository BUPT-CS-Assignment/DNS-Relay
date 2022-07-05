#include <main.h>
#include <console.h>
#include <server.h>

int __DEBUG__ = 0;
Socket _dns_server;

int main(int argc, char* argv[])
{
    /* console args parse */
    consoleParse(argc, argv);

    /* dns-relay socket initialize */
    Socket relay_service;
    socketInit(&relay_service, DNS_RELAY_ADDR, 53);

    /* local dns-server addr initialize. just for easy init. SOCKET not used*/
    socketInit(&_dns_server, LOCAL_DNS_ADDR, 53);

    /* start dns-relay service */
    start(&relay_service);

#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}

