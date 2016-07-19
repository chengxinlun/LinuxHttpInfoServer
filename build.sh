#!/bin/sh
g++ -std=c++11 -fPIC -shared sys_info.cpp -o libSysInfo.so
g++ -std=c++11 -fPIC -shared -pthread http_server.cpp -ldl -o libHttpServer.so
g++ -std=c++11 -g -pthread main.cpp -ldl -o main.o
