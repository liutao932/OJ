    #pragma once
    #include <jsoncpp/json/json.h>
    #include <signal.h>
    #include <unistd.h>
    #include <vector>
    #include "compiler.hpp"
    #include "complie_run.hpp"
    #include "../comm/log.hpp"
    #include "../comm/util.hpp"
    #include "runner.hpp"
    namespace ns_compile_and_run
    {
        class ComplieAndRun
        {
        public:
            static std::string CodeToDesc(int  code, const std::string &file_name)
            {
                std::string desc;
                switch (code)
                {
                case 0:
                    desc = "编译运行成功";
                    break;
                case -1:
                    desc = "用户提交的代码为空";
                    break;
                case -2:
                    desc = "未知错误";
                    break;
                case -3:
                    // desc = "代码编译是出现错误";
                    ns_util::FileUtil::ReadFile(ns_util::PathUtil::Error(file_name), &desc, true);
                    break;
                case SIGABRT: // 6
                    desc = "内存超过范围";
                    break;
                case SIGXCPU: // 24
                    desc = "CPU使用超时";
                    break;
                case SIGFPE:             // 8
                    desc = "浮点数溢出"; //除0
                    break;
                default:
                    desc = "未知错误" + std::to_string(code);
                    break;
                }
                return desc;
            }

        public: 
            static void RemoveTempFile(const std::string &file_name)
            {
                std::vector<std::string> AllTempFile{ns_util::PathUtil::Src(file_name),
                                                    ns_util::PathUtil::Error(file_name),
                                                    ns_util::PathUtil::Exe(file_name),
                                                    ns_util::PathUtil::Stderr(file_name),
                                                    ns_util::PathUtil::Stdin(file_name),
                                                    ns_util::PathUtil::Stdout(file_name)};
                for(const auto &e :AllTempFile)
                {
                    if(ns_util::FileUtil::IsFileExists(e));
                    unlink(e.c_str());
                }
            }
            /*
                输入：
                code:用户给自己提交的代码
                input:用户给自己的代码对应的输入，不作处理(后期可以扩展)
                cpu_limit:时间复杂度
                mem_limit:时间复杂度
                输出：
                status:状态码(必填)
                reason:请求结果(必填)
                stdout:程序运行结果(选填)
                stderr:程序运行完的错误结果(选填)

            */
            //参数
            // in_json:{"code":"";"input":"";"cpu_limit":"";"mem_limit":"";}
            // out_json:{"status":"0";"reason":"";"stdout":"";"stderr":""}
            static void Start(const std::string &in_json, std::string *out_json)
            {
                Json::Value in_vaule;
                Json::Reader reader;
                reader.parse(in_json, in_vaule); //最后在差错处理

                std::string code = in_vaule["code"].asString();
                std::string input = in_vaule["input"].asString();
                int cpu_limit = in_vaule["cpu_limit"].asInt();
                int mem_limit = in_vaule["mem_limit"].asInt();
                Json::Value out_vaule;

                int status_code = 0;
                std::string file_name;
                int run_result = 0;
                if (code.size() == 0)
                {
                    // //最后差错处理
                    // out_vaule["status"] = -1; //代码为空
                    // out_vaule["reason"] = "用户提交的代码是空的";
                    // //序列化
                    // return;
                    status_code = -1;
                    goto END;
                }
                //形成唯一文件名 毫秒级时间戳 + 原子性递增唯一值
                file_name = ns_util::FileUtil::UniqFileName();
                if (!ns_util::FileUtil::WiterFile(ns_util::PathUtil::Src(file_name), code)) //形成临时源src文件
                {
                    // out_vaule["status"] = -2; //未知错误
                    // out_vaule["reason"] = "提交的代码发生了未知错误";
                    // //序列化
                    // return;
                    status_code = -2;
                    goto END;
                }
                if (!ns_compiler::Compiler::Compile(file_name)) //编译失败
                {
                    // out_vaule["status"] = -3;
                    // //编译失败的内容保存到了.error文件中，读取序列化
                    // out_vaule["reason"] = us_util::FileUtil::ReadFile(us_util::PathUtil::Error(file_name));
                    // //序列化
                    // return;
                    status_code = -3;
                    goto END;
                }
                run_result = ns_runner::Runner::Run(file_name, cpu_limit, mem_limit); //需要知道时间复杂度和空间复杂度
                if (run_result < 0)
                {
                    // out_vaule["status"] = -2; //未知错误
                    // out_vaule["reason"] = "发生了未知错误";
                    // //序列化
                    // return;
                    status_code = -2;
                    goto END;
                }
                else if (run_result > 0)
                {
                    // out_vaule["status"] = -4;  //运行时报错，收到信号
                    // out_vaule["reason"] = SignoToDesc(); //将信号转化成报错原因；
                    // //序列化
                    // return;
                    status_code = run_result;
                    goto END;
                }
                else
                {
                    // //运行成功
                    // out_vaule["status"] = 0;
                    // out_vaule["reason"] = "运行成功";
                    status_code = 0;
                }
            END:
                out_vaule["status"] = status_code;
                out_vaule["reason"] = CodeToDesc(status_code,file_name);
                if (status_code == 0)
                {
                    //全部成功
                    std::string _stdout;
                    ns_util::FileUtil::ReadFile(ns_util::PathUtil::Stdout(file_name), &_stdout, true);
                    out_vaule["stdout"] = _stdout;
                // std::cout<<"标准输出："<<_stdout<<std::endl;
                    std::string _stderr;
                    ns_util::FileUtil::ReadFile(ns_util::PathUtil::Stderr(file_name), &_stderr, true);
                    out_vaule["stderr"] = _stderr;
                }
                //序列化
                Json::StyledWriter writer;
                *out_json = writer.write(out_vaule);
                RemoveTempFile(file_name);
            }
        };
    }