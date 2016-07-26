#include "function_base.h"
#include <ctime>
#include <string>
#include <utility>


class FuncTime: public FuncBase
{
    public:
        FuncTime();
        ~FuncTime(){};
        static std::pair<std::string, std::string> link()
        {
            return std::make_pair(func_name, lib_name);
        }
        std::string getResult()
        {
            return os_time;
        }
    private:
        std::string os_time;
        static const std::string lib_name;
        static const std::string func_name;
};


const std::string FuncTime::lib_name = "libTime";
const std::string FuncTime::func_name = "time";


FuncTime::FuncTime()
{
    time_t now = time(0);
    struct tm tstruct = *localtime(&now);
    char buf[256];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    os_time = buf;
}


extern "C" FuncBase* create_func()
{
    return new FuncTime();
}


extern "C" void destroy_func(FuncBase* object)
{
    delete object;
}


extern "C" std::pair<std::string, std::string> getLink()
{
    return FuncTime::link();
}
