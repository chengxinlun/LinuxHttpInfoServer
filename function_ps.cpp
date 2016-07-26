#include "function_base.h"
#include <ctime>
#include <string>
#include <utility>


class FuncPs: public FuncBase
{
    public:
        FuncPs();
        ~FuncPs(){};
        static std::pair<std::string, std::string> link()
        {
            return std::make_pair(func_name, lib_name);
        }
        std::string getResult()
        {
            return ps_html;
        }
    private:
        std::string ps_html;
        static const std::string lib_name;
        static const std::string func_name;
};


const std::string FuncPs::lib_name = "libPs";
const std::string FuncPs::func_name = "process";


FuncPs::FuncPs()
{
    std::string temp = ::getSystemOutput("ps -eo pid,uname,pcpu,pmem,comm");
    std::vector<std::string> ps = ::split(temp, '\n');
    std::stringstream ss;
    for (size_t i = 0; i < ps.size(); i++)
    {
        ps[i] = ps[i].substr(ps[i].find("\t") + 1, -1);
        replaceAll(ps[i], " ", "</td><td>");
        ss << "<tr><td>" << ps[i] << "</td></tr>";
    }
    std::string res = ss.str();
    replaceAll(res, "<td></td>", ""); // Delete extra blank table cells
    ps_html = "<table>" + res + "</table>";
}


extern "C" FuncBase* create_func()
{
    return new FuncPs();
}


extern "C" void destroy_func(FuncBase* object)
{
    delete object;
}


extern "C" std::pair<std::string, std::string> getLink()
{
    return FuncPs::link();
}
