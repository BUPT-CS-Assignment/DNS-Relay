#include <main.h>
#include <map.h>
#include <console.h>
#include <server.h>

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
        consoleLog(DEBUG_L0, RED"> bind port %d failed\n", ntohs(server->_addr.sin_port));
        exit(-1);
    }

    consoleLog(DEBUG_L0, "> server start. debug level L%d\n", __DEBUG__);

    int fromlen = sizeof(struct sockaddr_in);

    for(;;)
    {
        /* udp wait for new connection */
        thread_args* args = malloc(sizeof(thread_args));
        args->server = server;
        args->buf_len = recvfrom(server->_fd, args->buf, BUFFER_SIZE, 0, (struct sockaddr*)&args->connect._addr, &fromlen);

        if(args->buf_len > 0)
        {
            threadCreate(connectHandle, args);
        }
        else
        {
            free(args);
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

    Packet* p = packetParse(args->buf, args->buf_len);
    if(p == NULL)
    {
        free(args);
        threadExit();
    }

    int ret;

    if(GET_QR(p->FLAGS) == 1)
    {
        /* recv from local dns server -- query result */
        consoleLog(DEBUG_L0, BOLDCYAN"> recv query result\n");

        /* check packet info */
        packetCheck(p);

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

        if(urlQuery(p, RECORDS, R_NUM) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, BOLDYELLOW"> query from dns server\n");

            /* add to map */
            uint16_t converted = addToMap(p->ID,&args->connect._addr);
            SET_ID(args->buf,&converted);

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
}

