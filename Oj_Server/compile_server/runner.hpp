#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"


namespace ns_runner
{
    class Runner
    {
    public:
        Runner() = default;
        ~Runner() = default;

    public:
        static void SetProcLimit(int cpu_limit,int mem_limit)
        {
            //设置CPU时长
            struct rlimit _cpu_limit;
            _cpu_limit.rlim_max = RLIM_INFINITY;
            _cpu_limit.rlim_cur = cpu_limit;
            setrlimit(RLIMIT_CPU,&_cpu_limit);

            //设置内存大小

             struct rlimit _mem_limit;
             _mem_limit.rlim_max = RLIM_INFINITY;
             _mem_limit.rlim_cur = mem_limit*1024; //转化成KB
             setrlimit(RLIMIT_AS,&_mem_limit);
        }
        //返回值 > 0 ,程序异常，收到信号，返回值就是信号编号
        //返回值 == 0 ，正常运行完毕，结果保存到对应的临时文件
        //返回值 < 0 ,内部错误
        //cpu_limit:该程序运行的时候，可以使用的最大CPU资源上限
        //mem_limit:该程序运行的时候，可以使用最大内存
        static int Run(const std::string &file_name,int cpu_limit,int mem_limit)
        {
            //只考虑是否正确运行，不考虑结果是否正确
            /*
            一个程序在默认启动的时候
            标准输入：不处理
            标准输出：结果
            标准错误：运行时错误信息
            */
            std::string _exectue = ns_util::PathUtil::Exe(file_name);
            std::string _stdin = ns_util::PathUtil::Stdin(file_name);
            std::string _stdout = ns_util::PathUtil::Stdout(file_name);
            std::string _stderr = ns_util::PathUtil::Stderr(file_name);

            //打开临时文件
            umask(0);
            int _stdin_fd = open(_stdin.c_str(),O_CREAT|O_RDONLY,0644);
            int _stdout_fd = open(_stdout.c_str(),O_CREAT|O_WRONLY,0644);
            int _stderr_fd = open(_stderr.c_str(),O_CREAT|O_WRONLY,0644);
            if(_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                ns_log::LOG(ERROR)<<"运行时打开文件失败"<<"\n";
                return -1;  //文件打开失败
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                ns_log::LOG(ERROR)<<"运行时创建子进程失败"<<"\n";
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
            }
            else if (pid == 0)
            {
                dup2(_stdin_fd,0);
                dup2(_stdout_fd,1);
                dup2(_stderr_fd,2);

                SetProcLimit(cpu_limit,mem_limit);
                execl(_exectue.c_str(),_exectue.c_str(),nullptr);
                exit(1);
            }
            else
            {
               // std::cout<<"关闭文件描述符"<<std::endl;
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                int status = 0;
                //进程异常收到信号
                waitpid(pid, &status,0);
                ns_log::LOG(INFO)<<"运行完毕,info:"<< (status & 0x7F) << "\n";
                return status & 0x7F;
            }
            return 0;
        }
    };
}