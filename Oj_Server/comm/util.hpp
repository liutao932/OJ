#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <atomic>
#include <fstream>
#include <boost/algorithm/string.hpp>
namespace ns_util
{
    const std::string temp_path = "./temp/";
    //编译时需要的临时文件
    const std::string cpp = ".cpp";
    const std::string exe = ".exe";
    const std::string compile_error = ".error";

    //运行时需要的临时文件

    const std::string _stdin  = ".stdin";
    const std::string _stdout = ".stdout";
    const std::string _stderr = ".stderr";

    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }
        //构建源文件路劲+后缀的完整文件名
        // 123 -> ./temp/123.hpp
        //编译时需要的临时文件
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, cpp);
        }
        //构建可执行程序的完整路径+后缀名
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, exe);
        }
        static std::string Error(const std::string &file_name)
        {
            return AddSuffix(file_name,compile_error);
        }

        //运行时需要的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, _stdin);
        }
        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, _stdout);
        }
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, _stderr);
        }
    };
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time,nullptr);
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };
    class FileUtil
    {
    public:
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                //获取文件成功，文件存在
                return true;
            }
            return false;
        }
        static std::string UniqFileName()
        {
            //TODO
            static std::atomic_uint id(0);
            id++;
            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }
        static bool WiterFile(const std::string &target,const std::string &code)
        {
            //TODO
            std::ofstream out(target);
            if(!out.is_open())
            {
                return false;
            }
            out.write(code.c_str(),code.size());
            out.close();
            return true;
        }
        static bool ReadFile(const std::string &target,std::string *content,bool keep = false) 
        {
            //TODO
            (*content).clear();
           // std::cout<<"---------------------------------------"<<std::endl;
            std::ifstream in(target);
            if(!in.is_open())
            {
                return false;
            }   
            std::string line;
            //getline：不能保存行分割符，有些时候需要保留\n；
            while(std::getline(in,line))
            {
                (*content) += line;
                (*content) += keep ? "\n":"";
            }
            //std::cout<<"line"<<"---------------------------------------"<<std::endl;
            in.close();
            return true;
        }
    };
    class StringUtil
    {
    public:
        static void SplitString(const std::string &src,std::vector<std::string> *target,const std::string &sep)
        {
            //字符窜切分
            boost::split((*target),src,boost::is_any_of(sep),boost::algorithm::token_compress_on);

        }
    };
}