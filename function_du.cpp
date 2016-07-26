#include "function_base.h"
#include <string>


class FuncDu: public FuncBase
{
    public:
        FuncDu();
        ~FuncDu(){};
        static std::pair<std::string, std::string> link()
        {
            return std::make_pair(func_name, lib_name);
        }
        std::string getResult()
        {
            return os_du;
        }
    private:
        std::string os_du;
        static const std::string lib_name;
        static const std::string func_name;
};


const std::string FuncDu::lib_name = "libDu";
const std::string FuncDu::func_name = "disk";


FuncDu::FuncDu()
{
    std::string temp = ::getSystemOutput("df -hl");
    std::vector<std::string> du = ::split(temp, '\n');
    std::stringstream ss;
    for (size_t i = 0; i < du.size(); i++)
    {
        du[i] = du[i].substr(du[i].find("\t") + 1, -1);
        replaceAll(du[i], " ", "</td><td>");
        ss << "<tr><td>" << du[i] << "</td></tr>";
    }
    std::string res = ss.str();
    replaceAll(res, "<td></td>", ""); // Delete extra blank table cells
    os_du = "<table>" + res + "</table>";
}


// Interface for dynamic lib linkage
extern "C" FuncBase* create_func()
{
    return new FuncDu();
}


extern "C" void destroy_func(FuncBase* object)
{
    delete object;
}


extern "C" std::pair<std::string, std::string> getLink()
{
    return FuncDu::link();
}
