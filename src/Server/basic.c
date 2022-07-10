#include "console.h"
#include "server.h"


/* --------------------------------- Basic Global Variables ---------------------------------*/
LRU_cache* __URL_CACHE__ = NULL;
hash        __HOST_HASHMAP__;

char        __LOCAL_DNS_ADDR__[64] = "114.114.114.114";
char        __HOST_DEST__[255] = "host.txt";

int         __THREAD__ = 0;
int         __DEBUG__ = 0;
int         __CACHE_SCAN_TIME__ = 60;

Socket      __DNS_SERVER__;

void* cacheScanHandle();    //cache scan thread handler


/* --------------------------------- Main Function ---------------------------------*/
/**
 * @brief Start dns_realy  server
 *
 * @param server Socket pointer
 */
void start(Socket* server)
{
    if(server == NULL)
    {
        consoleLog(DEBUG_L0, "> no server created.\n");
        exit(-1);
    }

    /* bind port */
    if(bind(server->_fd, (struct sockaddr*)&server->_addr, sizeof(server->_addr)) < 0)
    {
        consoleLog(DEBUG_L0, RED"> bind port %d failed. code %d.\n", ntohs(server->_addr.sin_port), ERROR_CODE);
        exit(-1);
    }

    /* cache service init */
    if(cacheInit(&__URL_CACHE__) != LRU_OP_SUCCESS)
    {
        consoleLog(DEBUG_L0, RED"> cache service error.\n");
        exit(-1);
    }

    /* host service init */
    if(hostInit(&__HOST_HASHMAP__) != 0)
    {
        consoleLog(DEBUG_L0, RED"> no host file.\n");
    }

    consoleLog(DEBUG_L0, BOLDWHITE"> cache service start. cache capacity: %d.\n", __CACHE_LEN__);
    consoleLog(DEBUG_L0, BOLDWHITE"> server start. debug level L%d.\n", __DEBUG__);
    consoleLog(DEBUG_L0, BOLDWHITE"> local dns server: %s.\n", __LOCAL_DNS_ADDR__);

    /* start detached  debug thread && cache scan thread */
    threadDetach(threadCreate(cacheScanHandle, NULL));
    threadDetach(threadCreate(debugHandle, NULL));

    setTimeOut(server, 2, 0);   //set server Socket send time-out (2s) 

    int fromlen = sizeof(struct sockaddr_in);
    fd_set fds;

    for(;;)
    {
        FD_ZERO(&fds);
        FD_SET(server->_fd, &fds);
        SOCKET ret = select(server->_fd + 1, &fds, NULL, NULL, NULL);       //select usable socket
        if(ret <= 0)
        {
            continue;
        }
        if(FD_ISSET(server->_fd, &fds))
        {
            /* udp wait receive */
            thread_args* args = malloc(sizeof(thread_args));
            args->server = server;
            args->buf_len = recvfrom(server->_fd, args->buf, BUFFER_SIZE, 0, (struct sockaddr*)&args->connect._addr, &fromlen);

            if(args->buf_len > 0)
            {
                thread_t t_num = threadCreate(connectHandle, (void*)args);
                threadDetach(t_num);
            }
            else
            {
                free(args);
            }
        }

    }

    /* close server */
    socketClose(server);

}



/* --------------------------------- Thread Function ---------------------------------*/
/**
 * @brief new connect thread handler
 *
 * @param param new thread param
 * @return void*
 */
void* connectHandle(void* param)
{

    /* thread params parse */
    thread_args* args = (thread_args*)param;

    Socket* server = args->server;      //dns-relay server
    Socket* from = &args->connect;      //dns reqeust/response from

    /* packet parse */
    Packet* p = packetParse(args->buf, args->buf_len);

    if(p == NULL)
    {
        free(args);
        threadExit(1);
    }

    int ret = -1;

    if(GET_QR(p->FLAGS) == 1)
    {
        /* recv from local dns server -- query result */
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query result.\n");

        /* check packet info */
        packetCheck(p);

        /* add to cache */
        consoleLog(DEBUG_L0, BOLDMAGENTA"> cache length: %d.\n", urlStore(p));


        uint16_t origin;
        struct sockaddr_in from;

        /* query addr */
        if(queryMap(p->ID, &origin, &from) == 0)
        {
            SET_ID(args->buf, &origin);
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)&from, sizeof(from));
        }
    }
    else
    {
        /* recv from client -- query request */
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query request.\n");
        /* check packet info */
        packetCheck(p);

        if(urlQuery(p) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, BOLDYELLOW"> query from dns server.\n");

            /* add to map */
            uint16_t converted = addToMap(p->ID, &args->connect._addr);

            if(converted == UINT16_MAX)
            {
                consoleLog(DEBUG_L0, RED"> relay service busy.\n");
                ret = 0;
            }
            else
            {
                SET_ID(args->buf, &converted);

                /* send to dns server */
                ret = sendto(server->_fd, args->buf, args->buf_len, 0,
                    (struct sockaddr*)&__DNS_SERVER__._addr, sizeof(struct sockaddr));
            }
        }
        else
        {
            consoleLog(DEBUG_L0, BOLDGREEN"> query OK. send back.\n");

            /* generate response package */
            int buff_len;
            char* buff = responseFormat(&buff_len, p);

            /* send back to client */
            ret = sendto(server->_fd, buff, buff_len, 0,
                (struct sockaddr*)&args->connect._addr, sizeof(args->connect._addr));
            free(buff);
        }
    }

    if(ret < 0)
    {
        consoleLog(DEBUG_L0, BOLDRED"> send failed. code %d.\n", ERROR_CODE);
    }

    /* mem free */
    packetFree(p);
    free(args);

    /* close socket */
    socketClose(&args->connect);
    threadExit(1);
}



/**
 * @brief timing thread for scanning cache
 *
 * @return void*
 */
void* cacheScanHandle()
{
    while(1)
    {

#ifdef _WIN32
        Sleep(1000 * __CACHE_SCAN_TIME__);
#else
        sleep(__CACHE_SCAN_TIME__);
#endif

        if(cacheScan(__URL_CACHE__) == 0)
        {
            consoleLog(DEBUG_L2, "> no overdue record.\n");
        }
    }
}



/**
 * @brief debug operation input while running
 *
 * @return void*
 */
void* debugHandle()
{
    char cmd[255];
    while(1)
    {
        printf(BOLDWHITE"> ");
        scanf("%[^\n]", &cmd);
        if(cmd[0] == 'd')
        {
            if(cmd[1] == '2') __DEBUG__ = DEBUG_L2;
            else if(cmd[1] == '1') __DEBUG__ = DEBUG_L1;
            else __DEBUG__ = DEBUG_L0;
            consoleLog(DEBUG_L0, BOLDCYAN"> debug level reset: L%d\n", __DEBUG__);
        }
        else if(cmd[0] == 't')
        {
            consoleLog(DEBUG_L0, BOLDRED"> thread num: %d\n", __THREAD__);
        }
        else if(strcmp(cmd, "cache") == 0)
        {
            cacheCheck(__URL_CACHE__);
        }
        else if(strcmp(cmd, "flush") == 0)
        {
            cacheFlush(__URL_CACHE__);
        }
        else if(strncmp(cmd, "scan", 4) == 0 && strlen(cmd) >= 6)
        {
            int time = atoi((char*)cmd + 5);
            __CACHE_SCAN_TIME__ = (time > 30 ? time : 30);
            consoleLog(DEBUG_L0, BOLDRED"> cache scan period: %ds\n", __CACHE_SCAN_TIME__);
        }
        else if(strncmp(cmd, "server", 6) == 0)
        {
            uint32_t res = inet_addr((char*)cmd + 7);
            if(res != INADDR_NONE)
            {
                __DNS_SERVER__._addr.sin_addr.s_addr = res;
                consoleLog(DEBUG_L0, BOLDCYAN"> local dns server reset: %s\n", (char*)cmd + 7);
            }
        }
        else
        {
            consoleLog(DEBUG_L0, BOLDRED"> command undefined.\n");
        }
        rewind(stdin);
    }
}

