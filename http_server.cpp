#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cstring>
#include <csignal>
#include <unordered_map>
#include "http_partial.h"
#include "function_base.h"
#include "http_server.h"
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
    libLocator = new std::unordered_map<std::string, std::string>();
}


void HttpServer::getlibs()
{
    libLocator->clear();
    std::string all_libs = ::getSystemOutput("ls *.so");
    ::replaceAll(all_libs, "\n", ",");
    std::vector<std::string> libs_list = split(all_libs, ',');
    for (size_t i = 0; i < libs_list.size(); i ++)
    {
        libs_list[i] = "./" + libs_list[i];
        char temp[libs_list[i].length() + 1];
        strncpy(temp, libs_list[i].c_str(), sizeof(temp));
        temp[sizeof(temp) - 1] = 0;
        void* lib = dlopen(temp, RTLD_LAZY);
        if (lib == NULL)
        {
            {
                std::lock_guard<std::mutex> lock(cout_mtx);
                std::cout << "[Thread/Warn]: Unable to open library " << libs_list[i] << std::endl;
            }
            continue;
        }
        std::pair<std::string, std::string> (*linklib)();
        linklib = (std::pair<std::string, std::string>(*)())dlsym(lib, "getLink");
        if (linklib == NULL)
        {
            {
                std::lock_guard<std::mutex> lock(cout_mtx);
                std::cout << "[Thread/Warn]: Library " << libs_list[i] << " is malformed" << std::endl;
            }
            continue;
        }
        libLocator->insert(linklib());
        dlclose(lib);
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[Thread/Info]: Library " << libs_list[i] << " found and registered successfully" << std::endl;
        }
    }
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
        getlibs();
        pool->enqueue([&](int i){return this->service(i);}, new_socket_fd);
    }
}


void HttpServer::service(int new_socket_fd)
{
    char buffer[2048];
    bzero(buffer, 2048);
    int n = read(new_socket_fd, buffer, 2047);
    // Error return
    if (n < 0)
    {
        close(new_socket_fd);
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[Thread/Error]: Socket error (code: " << n << ") when estabilishing connection with client." << std::endl;
        }
        return;
    }
    // Null return
    if (n == 0)
    {
        close(new_socket_fd);
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[Thread/Info]: Connection closed by client" << std::endl;
        }
        return;
    }
    // Parse request
    Request* rq = new Request(buffer);
    std::string method = rq->getMethod();
    std::string uri = rq->getUri();
    delete rq;
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[Thread/Info]: Client HTTP request: " << method << " " << uri << std::endl;
    }
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
        // Get keys
        std::vector<std::string> keys;
        keys.reserve(libLocator->size());
        for (auto it = libLocator->begin(); it != libLocator->end(); ++it)
        {
            keys.push_back(it->first);
        }
        // Serving
        std::string uri_chop = uri.substr(uri.find('/') + 1, uri.find('.'));
        if (std::find(keys.begin(), keys.end(), uri_chop) != keys.end()) // In keys
        {
            code = 200;
            rp = "OK";
            std::string lib_name = "./" + (*libLocator)[uri_chop] + ".so";
            char temp[lib_name.length() + 1];
            strncpy(temp, lib_name.c_str(), sizeof(temp));
            temp[sizeof(temp) - 1] = 0;
            // Load library
            void* lib = dlopen(temp, RTLD_LAZY);
            FuncBase* (*create)();
            void (*destroy)(FuncBase*);
            create = (FuncBase* (*)())dlsym(lib, "create_func");
            destroy = (void (*)(FuncBase*))dlsym(lib, "destroy_func");
            FuncBase* base = (FuncBase*)create();
            html = "<html>" + base->getResult() + "</html>";
            destroy(base);
            dlclose(lib);
        }
        else if (uri == "/index.html") // Index page
        {
            // Setting response code and reason phrase
            code = 200;
            rp = "OK";
            html = "<html><center>Welcome to this simple HTTP Linux managemenet server.<br>Please click on of the below links.<br>";
            for (auto it = libLocator->begin(); it != libLocator->end(); ++it)
                html = html + "<a href=\"/" + it->first + "\">" + it->first + "</a><br>";
            html = html + "<a href=\"/about.html\">About this server</a></center></html>";
        }
        else if (uri == "/about.html") // About this site page
        {
            // Setting response code and reason phrase
            code = 200;
            rp = "OK";
            //Create about page
            html = "<html><center>Simple Linux Info Display Server<br><br>";
            html = html + "Author: Cheng Xinlun, Department of Physics, Tsinghua University</br>Student ID: 2014012198<br>";
            html = html + "<br>This server is programmed with c++ using several GNU c library and c++11 standard.<br>";
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
    temp[sizeof(temp) - 1] = 0;
    write(new_socket_fd, temp, sizeof(temp));
    delete rs;
    // Close connection with client
    close(new_socket_fd);
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[Thread/Info]: Connection closed with client" << std::endl;
    }
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


int main()
{
    HttpServer* hs = new HttpServer(50001);
    try
    {
        hs->launch();
    }
    catch (const HSException& e)
    {
        std::cout << "Terminating" << std::endl;
        hs->terminate();
    }
    delete hs;
    return 0;
}
