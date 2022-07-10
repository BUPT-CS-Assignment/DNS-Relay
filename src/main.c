#include "console.h"
#include "server.h"

int main(int argc, char* argv[])
{
    /* console args parse */
    consoleParse(argc, argv);

    /* dns-relay socket initialize */
    Socket relay_service;
    socketInit(&relay_service, INADDR_ANY , 53);

    /* local dns-server addr initialize. just for easy init. SOCKET not used*/
    socketInit(&__DNS_SERVER__, inet_addr(__LOCAL_DNS_ADDR__), 53);

    /* start dns-relay service */
    start(&relay_service);

#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}

