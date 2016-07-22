#ifndef SYS_INFO_H
#define SYS_INFO_H

#include <string>
#include <vector>


class OsInfo
{
    public:
        OsInfo();
        ~OsInfo();
        virtual std::string get_dis_ver(){return version;};
        virtual std::string get_kernel(){return kernel;};
        virtual std::string get_time(){return os_time;};
        virtual std::string get_du(const char* path);
        virtual std::string get_ps();

    private:
        std::string version;
        std::string kernel;
        std::string os_time;
};
#endif
