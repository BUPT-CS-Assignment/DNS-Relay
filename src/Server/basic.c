#include "utils/map.h"
#include "console.h"
#include "server.h"
#include "file.h"

LRU_cache* _url_cache = NULL;
char _local_dns_addr[64] = "114.114.114.114";
int          __THREAD__ = 0;
hash _hash_map;

/**
 * @brief Start dns_realy  server
 *
 * @param server Socket pointer
 */
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
        consoleLog(DEBUG_L0, RED"> bind port %d failed. code %d\n", ntohs(server->_addr.sin_port), ERROR_CODE);
        exit(-1);
    }

    /* cache service init */
    if(LRU_cache_init(&_url_cache) != LRU_OP_SUCCESS)
    {
        consoleLog(DEBUG_L0, RED"> cache service error.\n");
        exit(-1);
    }

    if(file_init(&_hash_map) != 0)
    {
        consoleLog(DEBUG_L0, RED"> no host file.\n");
    }

    consoleLog(DEBUG_L0, BOLDWHITE"> cache service start. cache capacity: %d\n", LRU_CACHE_LENGTH);
    consoleLog(DEBUG_L0, BOLDWHITE"> server start. debug level L%d\n", __DEBUG__);
    consoleLog(DEBUG_L0, BOLDWHITE"> local dns server: %s\n", _local_dns_addr);

    thread_t t_fd = threadCreate(debugHandle, NULL);
    threadDetach(t_fd);

    setTimeOut(server, 2, 0);

    int fromlen = sizeof(struct sockaddr_in);
    fd_set fds;

    for(;;)
    {
        FD_ZERO(&fds);
        FD_SET(server->_fd, &fds);
        SOCKET ret = select(server->_fd + 1, &fds, NULL, NULL, NULL);
        if(ret <= 0)
        {
            continue;
        }
        if(FD_ISSET(server->_fd, &fds))
        {
            /* udp wait for new connection */
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
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query result\n");

        /* check packet info */
        packetCheck(p);

        /* add to cache */
        consoleLog(DEBUG_L0, BOLDMAGENTA"> cache len %d\n", urlStore(p));


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
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query request\n");
        /* check packet info */
        packetCheck(p);

        if(urlQuery(p) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, BOLDYELLOW"> query from dns server\n");

            /* add to map */
            uint16_t converted = addToMap(p->ID, &args->connect._addr);
            SET_ID(args->buf, &converted);

            /* send to dns server */
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)&_dns_server._addr, sizeof(struct sockaddr));

        }
        else
        {
            consoleLog(DEBUG_L0, BOLDGREEN"> query OK. send back\n");

            /* generate response package */
            int buff_len;
            char* buff = responseFormat(&buff_len, p);

            /* send back to client */
            ret = sendto(server->_fd, buff, buff_len, 0, (struct sockaddr*)&args->connect._addr, sizeof(args->connect._addr));
            free(buff);
        }
    }

    if(ret < 0)
    {
        consoleLog(DEBUG_L0, BOLDRED"> send failed. code %d\n", ERROR_CODE);
    }

    /* mem free */
    packetFree(p);
    free(args);

    /* close socket */
    socketClose(&args->connect);
    threadExit(1);
}



/**
 * @brief debug thread handler
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
        if(strcmp(cmd, "cache") == 0)
        {
            LRU_cache_check(_url_cache);
        }
        else if(strncmp(cmd, "server", 6) == 0)
        {
            uint32_t res = inet_addr((char*)cmd + 7);
            if(res != INADDR_NONE)
            {
                _dns_server._addr.sin_addr.s_addr = res;
                consoleLog(DEBUG_L0, BOLDCYAN"> local dns server reset: %s\n", (char*)cmd + 7);
            }
        }
        else if(cmd[0] == 'd')
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
        else
        {
            consoleLog(DEBUG_L0, BOLDRED"> command undefined.\n");
        }
        rewind(stdin);
    }
}

