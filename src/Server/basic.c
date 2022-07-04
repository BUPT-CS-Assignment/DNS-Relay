#include <main.h>
#include <map.h>
#include <console.h>
#include <server.h>

typedef struct thread_args
{
    char buf[BUFFER_SIZE];
    Socket* server;
    Socket connect;
    int buf_len;

}thread_args;

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
#ifdef _WIN32


#else
    pthread_t pt;
    pthread_attr_t attr;
#endif

    int fromlen = sizeof(struct sockaddr_in);

    for(;;)
    {
        /* wait for new connection */
        thread_args* args = malloc(sizeof(thread_args));
        args->server = server;
        args->buf_len = recvfrom(server->_fd, &args->buf, BUFFER_SIZE, 0, (struct sockaddr*)&args->connect._addr, &fromlen);
        
        if(args->buf_len > 0)
        {

            /* new thread create */
#ifdef _WIN32
            HANDLE thread;
            thread = CreateThread(NULL,0,connectHandle,args,0,NULL);
            if(thread == NULL){
                consoleLog(DEBUG_L0,RED"> thread create failed. code %d\n",GetLastError());
                free(args);
            }
            CloseHandle(thread);
#else
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    //thread detached
            int ret = pthread_create(&pt, &attr, connectHandle, (void*)args);
            if(ret != 0){
                consoleLog(DEBUG_L0,RED"> thread create failed. code %d\n",ret);
                free(args);
            }
#endif

        }
        else
        {
            free(args);
        }
    }

    /* close server */
    socketClose(server);
    
}

void* connectHandle(void* param)
{

    /* thread params parse */
    thread_args* args = (thread_args*)param;

    Socket* server = args->server;      //dns-relay server
    Socket* from = &args->connect;      //dns reqeust/response from

    Packet* p = packetParse(args->buf, args->buf_len);
    if(p == NULL)
    {
        free(args);

#ifdef _WIN32
        Sleep(10);
        ExitThread(NULL);
#else
        usleep(10000);
        pthread_exit(0);
#endif

    }

    int ret;

    if(GET_QR(p->FLAGS) == 1)
    {
        /* recv from local dns server -- query result */
        consoleLog(DEBUG_L0, BOLDCYAN"> recv query result\n");

        /* check packet info */
        packetCheck(p);

        uint16_t id = p->ID;

        /* query addr */
        struct sockaddr_in* client = queryMap(&AddrMAP, p->ID);
        if(client != NULL)
        {
            /* send back */
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)client, sizeof(struct sockaddr_in));
            free(client);
        }
    }
    else
    {
        /* recv from client -- query request */
        consoleLog(DEBUG_L0,BOLDBLUE"> recv query request\n");
        /* check packet info */
        packetCheck(p);


        int ret;
        if(urlQuery(p, RECORDS, R_NUM) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, BOLDYELLOW"> query from dns server\n");

            /* query from local dns */
            struct sockaddr_in dns_addr;

            /* local dns server setting */
            dns_addr.sin_addr.s_addr = LOCAL_DNS_ADDR;
            dns_addr.sin_family = AF_INET;     //IPv4
            dns_addr.sin_port = htons(53);     //Port

            /* add to map */
            addToMap(&AddrMAP, p->ID, &args->connect._addr);
            //mapCheck(&AddrMAP);
            
            /* send to dns server */
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)&dns_addr, sizeof(dns_addr));

        }
        else
        {
            consoleLog(DEBUG_L0, BOLDGREEN"> query OK. send back\n");

            //Re-Parse
            //Packet *temp = packetParse(buf, len);
            //packetCheck(temp);
            
            /* generate response package */
            int buff_len;
            char* buff = responseFormat(&buff_len, p);
            
            /* send back to client */
            ret = sendto(server->_fd, buff, buff_len, 0, (struct sockaddr*)&args->connect._addr, sizeof(args->connect._addr));
            free(buff);
        }
    }
    
    if(ret < 0){
        consoleLog(DEBUG_L0,BOLDRED"> send failed. code %d\n",
#ifdef _WIN32
        GetLastError()
#else
        ret
#endif
        );
    }

    /* mem free */
    packetFree(p);
    free(args);
    
    /* close socket */
    socketClose(&args->connect);


#ifdef _WIN32
    Sleep(10);
    ExitThread(NULL);
#else
    usleep(10000);
    /* pthread exit */
    pthread_exit(0);
#endif
}

