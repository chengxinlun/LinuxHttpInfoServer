#include <sys/statvfs.h>
#include <fstream>
#include <iostream>
#include <sys/utsname.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "sys_info.h"


OsInfo::OsInfo()
{
    // Get distributor id
    FILE *disp_id;
    disp_id = popen("/usr/bin/lsb_release -i 2>&1", "r");
    char id_temp1[100];
    std::string id_temp2 = "";
    while (fgets(id_temp1, sizeof(id_temp1), disp_id) != NULL)
        id_temp2 = id_temp2 + id_temp1;
    pclose(disp_id);
    std::string id = id_temp2.substr(id_temp2.find("\t") + 1, id_temp2.length());
    id = id.erase(id.length() - 1);
    // Get release version
    FILE *release;
    release = popen("/usr/bin/lsb_release -r 2>&1", "r");
    char re_temp1[100];
    std::string re_temp2 = "";
    while (fgets(re_temp1, sizeof(re_temp1), release) != NULL)
            re_temp2 = re_temp2 + re_temp1;
    pclose(release);
    std::string re = re_temp2.substr(re_temp2.find("\t") + 1, re_temp2.length());
    re = re.erase(re.length() - 1);
    // Version = distributor id + release
    version = id + " " + re;   
    // Get kernel information
    struct utsname sys_info;
    uname(&sys_info);
    kernel = sys_info.release;
    // Get system time
    time_t now = time(0);
    struct tm tstruct = *localtime(&now);
    char buf[80];
    // Parse time into string
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    os_time = buf;
}


OsInfo::~OsInfo()
{
}


std::string OsInfo::get_du(const char* path)
{
    struct statvfs du;
    int n = statvfs(path, &du);
    if (n == -1)
        return "Cannot get disk usage.";
    else
    {
        double fs_g = du.f_bsize * du.f_bfree / (1024.0 * 1024.0 * 1024.0);
        double fs_r = du.f_bfree / du.f_blocks * 100.0;
        std::stringstream res;
        res << std::fixed << std::setprecision(2) << fs_g << "G (" << fs_r << "\%)";
        return res.str();
    }
}

extern "C" OsInfo* create_osinfo()
{
    return new OsInfo();
}


extern "C" void destroy_osinfo(OsInfo* object)
{
    delete object;
}
