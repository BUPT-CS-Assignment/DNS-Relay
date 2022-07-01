#include <main.h>
#include <map.h>
#include <console.h>
#include <server.h>

typedef struct pthread_args
{
    char buf[BUFFER_SIZE];
    Socket* server;
    Socket connect;
    int buf_len;
}pthread_args;

void start(Socket* server)
{
    if(server == NULL)
    {
        consoleLog(DEBUG_L0, "> no server created\n");
        exit(-1);
    }

    /* bind port */
    if(bind(server->_fd, (struct sockaddr*)&server->_addr, sizeof(server->_addr)) < 0)
    {
        consoleLog(DEBUG_L0, RED"> bind port %d failed\n", ntohs(server->_addr.sin_port));
        exit(-1);
    }

    consoleLog(DEBUG_L0, "> server start. debug level L%d\n",__DEBUG__);

    /* pthread args */
    pthread_t pt;
    pthread_attr_t attr;
    
    int fromlen = sizeof(struct sockaddr_in);

    for(;;)
    {
        /* wait for new connection */
        pthread_args* pta = malloc(sizeof(pthread_args));
        pta->server = server;
        pta->buf_len = recvfrom(server->_fd, &pta->buf, BUFFER_SIZE, 0, (struct sockaddr*)&pta->connect._addr, &fromlen);
        
        if(pta->buf_len > 0)
        {
            /* new thread create */
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //thread detached
            int ret = pthread_create(&pt, &attr, connectHandle, (void*)pta);
            if(ret != 0){
                consoleLog(DEBUG_L0,RED"> thread create failed. code %d\n",

#ifdef _WIN32
                GetLastError()
#else
                ret
#endif
                
                );
                
                free(pta);
            }
        }
        else
        {
            free(pta);
        }
    }

    /* close server */
    socketClose(server);
    
}

void* connectHandle(void* param)
{

    /* thread params parse */
    pthread_args* pta = (pthread_args*)param;

    Socket* server = pta->server;
    Socket* from = &pta->connect;

    /* set timeout */
    //setTimeOut(&client,10000,10000);

    Packet* p = packetParse(pta->buf, pta->buf_len);
    if(p == NULL)
    {
        free(pta);
        pthread_exit(0);
    }

    /* check packet info */
    packetCheck(p);

    if(GET_QR(p->FLAGS) == 1)
    {
        /* recv from local dns server -- query result */
        consoleLog(DEBUG_L0, "> recv from local dns\n");
        uint16_t id = p->ID;
        //memset(pta->buf, id, sizeof(uint16_t));

        /* query addr */
        struct sockaddr_in* client = queryMap(&AddrMAP, p->ID);
        if(client != NULL)
        {
            /* send back */
            sendto(server->_fd, pta->buf, pta->buf_len, 0, (struct sockaddr*)client, sizeof(struct sockaddr_in));
            free(client);
        }
    }
    else
    {
        /* recv from client -- query request */
        int ret;
        if(urlQuery(p, RECORDS, R_NUM) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, "> query from local dns server\n");
            /* query from local dns */
            struct sockaddr_in dns_addr;

            /* local dns server setting */
            dns_addr.sin_addr.s_addr = LOCAL_DNS_ADDR;
            dns_addr.sin_family = AF_INET;     //IPv4
            dns_addr.sin_port = htons(53);     //Port

            /* add to map */
            //uint16_t id = !p->ID;
            //memset(pta->buf, id, sizeof(uint16_t));
            addToMap(&AddrMAP, p->ID, &pta->connect._addr);
            mapCheck(&AddrMAP);
            /* send to dns server */
            ret = sendto(server->_fd, pta->buf, pta->buf_len, 0, (struct sockaddr*)&dns_addr, sizeof(dns_addr));

        }
        else
        {
            consoleLog(DEBUG_L0, "> query OK. send back\n");
            //Re-Parse
            //Packet *temp = packetParse(buf, len);
            //packetCheck(temp);
            
            /* generate response package */
            int buff_len;
            char* buff = responseFormat(&buff_len, p);
            
            /* send back to client */
            ret = sendto(server->_fd, buff, buff_len, 0, (struct sockaddr*)&pta->connect._addr, sizeof(pta->connect._addr));
            free(buff);
        }
        if(ret != 0){
            consoleLog(DEBUG_L0,"> send to server failed. code %d\n",
#ifdef _WIN32
            GetLastError()
#else
            ret
#endif
            );
        }
    }

    /* mem free */
    packetFree(p);
    free(pta);
    
    /* close socket */
    socketClose(&pta->connect);


#ifdef _WIN32
    Sleep(10);
#else
    usleep(10000);
#endif

    /* pthread exit */
    pthread_exit(0);

}

