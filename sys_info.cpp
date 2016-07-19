#include <fstream>
#include <iostream>
#include <sys/utsname.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
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


extern "C" OsInfo* create_object()
{
    return new OsInfo();
}


extern "C" void destroy_object(OsInfo* object)
{
    delete object;
}
