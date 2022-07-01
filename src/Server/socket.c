#include <main.h>
#include <console.h>
#include <server.h>

int socketInit(Socket* server,uint32_t address ,uint16_t port)
{
    /* Pointer Check */
    if(server == NULL)
    {
        return SOCKET_ERROR;
    }

    server->_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if((int)server->_fd < 0){
        consoleLog(DEBUG_L0, RED"socket error");
        exit(-1);
    }

    /* socket base */
    server->_addr.sin_addr.s_addr = address;
    server->_addr.sin_family = AF_INET;     //IPv4
    server->_addr.sin_port = htons(port);   //Port

    /* port reuse */
    int temp = 1;
    setsockopt(server->_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(temp));

    return 0;
}

void socketClose(Socket* s)
{
    if(s == NULL)
    {
        return;
    }
#ifdef _WIN32
    closesocket(s->_fd);
#else 
    close(s->_fd);
#endif
}


void setTimeOut(Socket* s, uint32_t send_timeout, uint32_t recv_timeout)
{
    if(s == NULL)
    {
        return;
    }

    /* set send timeout */
    setsockopt(s->_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&send_timeout, sizeof(int));

    /* set recv timeout */
    setsockopt(s->_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout, sizeof(int));

}