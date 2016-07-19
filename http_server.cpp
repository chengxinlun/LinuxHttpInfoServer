#include <iostream>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cstring>
#include "http_partial.h"
#include "http_server.h"
#include "sys_info.h"


HttpServer::HttpServer(unsigned short port)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connection from any ip
    server_addr.sin_port = htons(port);
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)))
        throw std::runtime_error("Unable to bind to designated port.");
}


void HttpServer::launch()
{
    std::cout << "Press Ctrl + C to exit." << std::endl;
    listen(socket_fd, 5);
    // When request is received
    socklen_t client_len;
    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);
    int new_socket_fd;
    new_socket_fd = accept(socket_fd, (struct sockaddr*) &client_addr, & client_len);
    service(new_socket_fd);
}


void* HttpServer::service(int new_socket_fd)
{
    char buffer[2048];
    while (true)
    {
        bzero(buffer, 2048);
        int n = read(new_socket_fd, buffer, 2047);
        // Error return
        if (n < 0)
        {
            close(new_socket_fd);
            throw std::runtime_error("Socket error");
        }
        // Null return
        if (n == 0)
            break;
        // Parse request
        Request* rq = new Request(buffer);
        std::string method = rq->getMethod();
        std::string uri = rq->getUri();
        delete rq;
        if (method != "GET")
        {
            // Return error
            Response* rs = new Response(400, "Bad Request", "Bad Request");
            std::string rss = rs->toString();
            char temp[rss.length() + 1];
            strncpy(temp, rss.c_str(), sizeof(temp));
            temp[sizeof(temp)] = 0;
            write(new_socket_fd, temp, sizeof(temp));
            delete rs;
        }
        else
        {
            // Serving
            // Load libSysInfo.so
            void* libsysinfo = dlopen("./libSysInfo.so", RTLD_LAZY);
            // Define constructor and deconstructor
            OsInfo* (*create)();
            void (*destroy)(OsInfo*);
            create = (OsInfo* (*)())dlsym(libsysinfo, "create_object");
            destroy = (void(*)(OsInfo*))dlsym(libsysinfo, "destroy_object");
            // Create object
            OsInfo* os_info = (OsInfo*)create();
            // Create html webpages
            std::string html 
                = "<html><center><table><tr><td>Time</td><td>" + os_info->get_time() + 
                "</td></tr><tr><td>Distribution</td><td>" + os_info->get_dis_ver() +
                "</td></tr><tr><td>Kernal</td><td>" + os_info->get_kernel() + 
                "</td></tr></table></center></html>";
            // Delete object and unload libSysInfo.so
            destroy(os_info);
            dlclose(libsysinfo);
            // Create http response
            Response* rs = new Response(200, "OK", html);
            std::string rss = rs->toString();
            char temp[rss.length() + 1];
            strncpy(temp, rss.c_str(), sizeof(temp));
            temp[sizeof(temp)] = 0;
            write(new_socket_fd, temp, sizeof(temp));
            delete rs;
        }
    }
    close(new_socket_fd);
    return NULL;
}


void HttpServer::terminate()
{
    close(socket_fd);
}


HttpServer::~HttpServer()
{
}


// Interface for dynamic lib linkage
extern "C" HttpServer* create_object()
{
    return new HttpServer(50001);
}


extern "C" void destroy_object(HttpServer* object)
{
    delete object;
}
