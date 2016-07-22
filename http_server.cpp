#include <iostream>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cstring>
#include <csignal>
#include "http_partial.h"
#include "http_server.h"
#include "sys_info.h"
#include "thread_pool.h"


HttpServer::HttpServer(unsigned short port)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connection from any ip
    server_addr.sin_port = htons(port);
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)))
        throw std::runtime_error("Unable to bind to designated port.");
    pool = new ThreadPool(4);
}


void HttpServer::launch()
{
    signal(SIGINT, evac);
    std::cout << "Press Ctrl + C to exit." << std::endl;
    while (true)
    {
        listen(socket_fd, 5);
        socklen_t client_len;
        struct sockaddr_in client_addr;
        client_len = sizeof(client_addr);
        int new_socket_fd;
        new_socket_fd = accept(socket_fd, (struct sockaddr*) &client_addr, & client_len);
        pool->enqueue([&](int i){return this->service(i);}, new_socket_fd);
    }
}


void HttpServer::service(int new_socket_fd)
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
        // Response parameters
        int code;
        std::string rp;
        std::string html;
        std::unordered_map<std::string, std::string>* extra_header = new std::unordered_map<std::string, std::string>();
        // Crafting detailed response
        if (method != "GET")
        {
            // Method error
            code = 400;
            rp = "Bad request";
            html = "Bad request";
        }
        else
        {
            // Serving
            if (uri == "/index.html") // Index page
            {
                // Setting response code and reason phrase
                code = 200;
                rp = "OK";
                // Load libSysInfo.so
                void* libsysinfo = dlopen("./libSysInfo.so", RTLD_LAZY);
                // Define constructor and deconstructor
                OsInfo* (*create)();
                void (*destroy)(OsInfo*);
                create = (OsInfo* (*)())dlsym(libsysinfo, "create_osinfo");
                destroy = (void(*)(OsInfo*))dlsym(libsysinfo, "destroy_osinfo");
                // Create object
                OsInfo* os_info = (OsInfo*)create();
                // Create index page
                html 
                    = "<html><center><table><tr><td>Time: </td><td>" + os_info->get_time() + 
                    "</td></tr><tr><td>Distribution: </td><td>" + os_info->get_dis_ver() +
                    "</td></tr><tr><td>Kernel: </td><td>" + os_info->get_kernel() + 
                    "</td></tr><tr><td>Available Space: </td><td>/: " + os_info->get_du("/") + "<br>/home: " + os_info->get_du("/home") +
                    "</td></tr>" + os_info->get_ps() +
                    "</table></br><a href=\"/about.html\">About</a></center></html>";
                // Delete object and unload libSysInfo.so
                destroy(os_info);
                dlclose(libsysinfo);
            }
            else if (uri == "/about.html") // About this site page
            {
                // Setting response code and reason phrase
                code = 200;
                rp = "OK";
                //Create about page
                html = "<html><center>Simple Linux Info Display Server</br></br>";
                html = html + "Author: Cheng Xinlun, Department of Physics, Tsinghua University</br>Student ID: 2014012198</br>";
                html = html + "</br>This server is programmed with c++ using several GNU c library and c++11 standard.</br>";
                html = html + "Report bugs at <a href=\"https://github.com/chengxinlun/LinuxHttpInfoServer\">github</a>";
            }
            else if (uri == "/") // Redict / to /index.html
            {
                // Setting response code and reason phrase
                code = 302;
                rp = "Found";
                extra_header->insert(std::make_pair("Location", "/index.html"));
            }
            else
            {
                // Setting response code and reason phrase
                code = 404;
                rp = "Not Found";
                html = "<html><center>Don\'t worry. It is just a 404 error.</center></html>";
            }
        }
        // Create response
        Response* rs = new Response(code, rp, html);
        rs->add_header(extra_header);
        std::string rss = rs->toString();
        char temp[rss.length() + 1];
        strncpy(temp, rss.c_str(), sizeof(temp));
        temp[sizeof(temp)] = 0;
        write(new_socket_fd, temp, sizeof(temp));
        delete rs;
    }
    close(new_socket_fd);
    std::cout << "Connection closed with client" << std::endl;
    return;
}


void HttpServer::terminate()
{
    close(socket_fd);
}


HttpServer::~HttpServer()
{
    delete pool;
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
