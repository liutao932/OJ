#pragma once
#include <iostream>
#include <string>
#include "util.hpp"
namespace ns_log
{
    //日志等级
    enum
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };
    //LOG()<<"message"
    inline std::ostream &Log(const std::string &level,const std::string &file_name,const int line)
    {
        //添加日志等级
        std::string message = "[";
        message += level;
        message += "]";

        //添加报错文件名称
        message += "[";
        message += file_name;
        message += "]";

        //添加报错行
        message += "[";
        message += std::to_string(line);
        message += "]";

        //日志时间戳
        message += "[";
        message += ns_util::TimeUtil::GetTimeStamp();
        message += "]";


        std::cout << message;

        return std::cout;
    }

    //开放日志
    #define LOG(level) Log(#level,__FILE__,__LINE__)
}