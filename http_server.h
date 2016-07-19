#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include "thread_pool.h"


class HttpServer
{
    public:
        HttpServer(unsigned short port);
        ~HttpServer();
        virtual void launch();
        virtual void terminate();
        virtual void service(int new_socket_fd);
    private:
        int socket_fd;
        struct sockaddr_in server_addr;
        ThreadPool* pool;
        void* libthreadpool;
};
#endif
