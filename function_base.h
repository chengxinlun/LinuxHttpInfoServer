#ifndef FUNCTION_BASE_H
#define FUNCTION_BASE_H

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>


// Base class for all functionalities
class FuncBase
{
    public:
        FuncBase(){};
        virtual ~FuncBase(){};
        virtual std::string getResult(){};
};


// Some frequently used functions
std::string getSystemOutput(const char* command)
{
    FILE *command_file;
    command_file = popen(command, "r");
    char temp[1024];
    std::string output = "";
    while (fgets(temp, sizeof(temp), command_file) != NULL)
        output = output + temp;
    pclose(command_file);
    return output;
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s,char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


void replaceAll(std::string& str, const std::string& from, const std::string& to) 
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) 
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}
#endif
