#include "console.h"
#include "main.h"


/**
 * @brief platform socket service init
 *
 * @return int
 */
int platformInit()
{
#ifdef _WIN32
    WSADATA w;
    return WSAStartup(MAKEWORD(2, 2), &w);
#endif
    return 0;
}



/**
 * @brief Basic sockaddr initialize
 * 
 * @param server Socket pointer
 * @param address inet address
 * @param port port to bind
 */
void sockaddrInit(Socket* server, uint32_t address, uint16_t port)
{
    /* Pointer Check */
    if(server == NULL)  return;

    /* socket base */
    memset(&server->_addr, 0, sizeof(struct sockaddr_in));
    server->_addr.sin_addr.s_addr = address;    //Connection Address
    server->_addr.sin_family = AF_INET;         //IPv4
    server->_addr.sin_port = htons(port);       //Port

}


/**
 * @brief Basic Socket Initialize
 * 
 * @param server Socket struct pointer
 * @param IPPROTO socket IPPROTO
 * @return int socket() return value
 */
int socketInit(Socket* server, int IPPROTO)
{
    /* Pointer Check */
    if(server == NULL)  return SOCKET_ERROR;

    /*  new socket */
    server->_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO);

    if((int)server->_fd < 0)
    {
        consoleLog(DEBUG_L0, RED"socket error");
        return -1;
    }

    /* port reuse */
    int temp = 1;
    setsockopt(server->_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(temp));

    return 0;

}



/**
 * @brief Close socket
 *
 * @param s SOCKET fd
 */
void socketClose(Socket* s)
{
    if(s == NULL)   return;

#ifdef _WIN32
    closesocket(s->_fd);
#else 
    close(s->_fd);
#endif

}



/**
 * @brief Set Socket TimeOut
 *
 * @param s Socket pointer
 * @param send_timeout send()/sendto() timeout
 * @param recv_timeout recv()/recvfrom() timeout
 */
void setTimeOut(Socket* s, uint32_t send_timeout, uint32_t recv_timeout)
{
    if(s == NULL)   return;
    struct timeval send_timeval;
    send_timeval.tv_sec = send_timeout;

    struct timeval recv_timeval;
    recv_timeval.tv_sec = recv_timeout;

    /* set send timeout */
    if(send_timeout != 0)
        setsockopt(s->_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&send_timeval, sizeof(send_timeval));

    /* set recv timeout */
    if(recv_timeout != 0)
        setsockopt(s->_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeval, sizeof(recv_timeval));

}