#ifndef SYS_INFO_H
#define SYS_INFO_H

class OsInfo
{
    public:
        OsInfo();
        ~OsInfo();
        std::string get_dis_ver(){return version;};
        std::string get_kernel(){return kernel;};
        std::string get_time(){return os_time;};

    private:
        std::string version;
        std::string kernel;
        std::string os_time;
};
#endif
