#!/bin/sh
g++ -std=c++11 -fPIC -shared sys_info.cpp -o libSysinfo.so
g++ -std=c++11 -fPIC -shared -pthread thread_pool.h -o libThreadPool.so
g++ -std=c++11 -fPIC -shared -pthread http_server.cpp -ldl -o libHttpServer.so
g++ -std=c++11 -g -pthread main.cpp -ldl -o main.o
