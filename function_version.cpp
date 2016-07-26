#include "function_base.h"
#include <ctime>
#include <string>
#include <utility>
#include <sys/utsname.h>


class FuncVersion: public FuncBase
{
    public:
        FuncVersion();
        ~FuncVersion(){};
        static std::pair<std::string, std::string> link()
        {
            return std::make_pair(func_name, lib_name);
        }
        std::string getResult()
        {
            return os_version;
        }
    private:
        std::string os_version;
        std::string id;
        std::string release;
        std::string kernel;
        static const std::string lib_name;
        static const std::string func_name;
};


const std::string FuncVersion::lib_name = "libVersion";
const std::string FuncVersion::func_name = "version";


FuncVersion::FuncVersion()
{
    // Distributor id
    id = ::getSystemOutput("/usr/bin/lsb_release -i");
    id = id.substr(id.find("\t") + 1, -1);
    id = id.erase(id.length() - 1);
    // Release version
    release = ::getSystemOutput("/usr/bin/lsb_release -r");
    release = release.substr(release.find("\t") + 1, -1);
    release = release.erase(release.length() - 1);
    // Kernel
    struct utsname kernel_info;
    uname(&kernel_info);
    kernel = kernel_info.release;
    // Parsing into a html table
    os_version = "<table><tr><td>Distribution: </td><td>" + id + " " + 
        release + "</td></tr><tr><td>Kernel Version: </td><td>" + kernel +
        "</td></tr></table>";
}


extern "C" FuncBase* create_func()
{
    return new FuncVersion();
}


extern "C" void destroy_func(FuncBase* object)
{
    delete object;
}


extern "C" std::pair<std::string, std::string> getLink()
{
    return FuncVersion::link();
}
