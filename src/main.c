#include "console.h"
#include "server.h"

int main(int argc, char* argv[])
{
    /* console args parse */
    consoleParse(argc, argv);

    /* platform initialize */
    if(platformInit() != 0)
    {
        consoleLog(DEBUG_L0, RED"> platform socket init failed.\n");
        exit(-1);
    }

    /* dns-relay socket initialize */
    Socket relay_service;
    sockaddrInit(&relay_service, INADDR_ANY, 53);
    socketInit(&relay_service, IPPROTO_UDP);

    /* local dns-server addr initialize.*/
    sockaddrInit(&__DNS_SERVER__, inet_addr(__LOCAL_DNS_ADDR__), 53);

    /* start dns-relay service */
    start(&relay_service);

#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}

