#!/bin/sh
g++ -std=c++11 -fPIC -shared function_ps.cpp -o libPs.so
g++ -std=c++11 -fPIC -shared function_time.cpp -o libTime.so
g++ -std=c++11 -fPIC -shared function_version.cpp -o libVersion.so
g++ -std=c++11 -fPIC -shared function_du.cpp -o libDu.so
g++ -std=c++11 -g -pthread http_server.cpp -ldl -o main.o
