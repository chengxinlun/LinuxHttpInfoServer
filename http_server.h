#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include "thread_pool.h"
#include <exception>


class HSException: public std::exception
{
    public:
        HSException(const char* detail):std::exception()
        {
             this->detail = detail;
        }
        const char* what() const throw()
        {
             return detail;
        }
    private:
        const char* detail;
};



class HttpServer
{
    public:
        HttpServer(unsigned short port);
        ~HttpServer();
        virtual void launch();
        virtual void terminate();
        virtual void service(int new_socket_fd);
        virtual void getlibs();
        static void evac(int sig)
        {
            HSException hse("ctrl-c");
            throw hse;
        }
    private:
        int socket_fd;
        struct sockaddr_in server_addr;
        ThreadPool* pool;
        std::mutex cout_mtx;
        std::unordered_map<std::string, std::string>* libLocator;
};
#endif
