#include <main.h>
#include <console.h>
#include <server.h>


/**
 * @brief Basic Socket Initialize
 * 
 * @param server    Socket struct pointer 
 * @param address   Socket sockaddr
 * @param port      Socket port
 * @return int      socket() return value
 */
int socketInit(Socket* server,uint32_t address ,uint16_t port)
{
    /* Pointer Check */
    if(server == NULL)  return SOCKET_ERROR;

    /* platform initialize special for windows */ 
#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
#endif 

    /*  new udp socket */
    server->_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if((int)server->_fd < 0){
        consoleLog(DEBUG_L0, RED"socket error");
        exit(-1);
    }

    /* socket base */
    server->_addr.sin_addr.s_addr = address;    //Connection Address
    server->_addr.sin_family = AF_INET;         //IPv4
    server->_addr.sin_port = htons(port);       //Port

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

    /* set send timeout */
    setsockopt(s->_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&send_timeout, sizeof(int));

    /* set recv timeout */
    setsockopt(s->_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout, sizeof(int));

}