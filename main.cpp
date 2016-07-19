#include <dlfcn.h>
#include "http_server.h"




int main()
{
    // Loading lib
    void* handle = dlopen("./libHttpServer.so", RTLD_LAZY);

    HttpServer* (*create)();
    void (*destroy)(HttpServer*);

    create = (HttpServer* (*)())dlsym(handle, "create_object");
    destroy = (void(*)(HttpServer*))dlsym(handle, "destroy_object");
    HttpServer* hs = (HttpServer*)create();
    hs->launch();
    hs->terminate();
    destroy(hs);
}
