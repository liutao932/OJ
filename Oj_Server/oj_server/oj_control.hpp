#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include <cassert>
#include <fstream>
#include <jsoncpp/json/json.h>
#include "oj_model.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include "oj_view.hpp"
#include "../comm/httplib.h"
namespace ns_control
{
    const std::string service_machine = "./conf/service_machine.conf";
    //提供服务的主机
    class Machine
    {
    public:
        std::string ip; //编译服务的ip
        int port;       //编译服务的端口
        uint64_t load;  //编译服务的负载
        std::mutex *mtx;  //mutex禁止拷贝的，使用指针来完成
    public:
        Machine()
        :ip("")
        ,port(0)
        ,load(0)
        ,mtx(nullptr)
        {

        }
        ~Machine()
        {

        }
    public:
        void IncLoad()  //提升负载
        {
            if(mtx) mtx->lock();
            ++load;
            if(mtx) mtx->unlock();
        }
        void DecLoad() //减少负载
        {
            if(mtx) mtx->lock();
            --load;
            if(mtx) mtx->unlock(); 
        }
        uint64_t Load() //获取负载
        {
            uint64_t _load = 0;
             if(mtx) mtx->lock();
            _load = load;
            if(mtx) mtx->unlock(); 

            return _load;
        }
    };
    //负载均衡模块
    class LoadBlance
    {
    private:
        std::vector<Machine> machines; //可以提供编译服务所有主机
        std::vector<int> online; //所有在线的主机
        std::vector<int> offline; //所有离线主机
        std::mutex mtx;           //保证LoadBlance数据安全
    public:
        LoadBlance()
        {
            assert(LoadConf(service_machine));
            ns_log::LOG(ns_log::INFO)<<" 加载 "<<service_machine<<" 成功 "<<"\n";
        }
        ~LoadBlance()
        {

        }
    public:
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if(!in.is_open())
            {
                ns_log::LOG(ns_log::FATAL)<<"加载配置:"<<machine_conf<<"文件失败"<<"\n";
                return false;
            }
            std::string line;
            while(getline(in,line))
            {
                std::vector<std::string> tokens;
                ns_util::StringUtil::SplitString(line,&tokens,":");

                if(tokens.size() != 2)
                {
                     ns_log::LOG(ns_log::WARNING) <<" 切分 "<<line<<" 失败 "<<"\n";
                     continue;
                }

                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }
            in.close();
            return true;
        }
        //id:输出型参数
        //m :输出型参数
        bool SmartChoice(int* id,Machine **m)
        {
            //1.使用选择好的主机(更新该主机的负载)
            //2.我们需要可能离线该主机
            mtx.lock();
            //负载均衡的算法
            //1.随机数 + hash
            //2.轮询 + hash
            int online_num = online.size();
            if(online_num == 0)
            {
                mtx.unlock();
                ns_log::LOG(ns_log::FATAL) << "后端编译服务全部挂掉了,请运维的老铁尽快查看"<<"\n";
                return false;
            }
            //通过编译找到负载最小的机器
            *id = online[0];
            *m =  &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for(int i = 0; i < online_num; ++i)
            {
                min_load = min_load < machines[online[i]].Load() ? machines[online[i]].Load() : min_load;
                *id = online[i];
                *m  = &machines[online[i]];
            }
            mtx.unlock();
            return true;
        }
        void OfflineMachine(int which)
        {
            mtx.lock();
            for(auto iter = online.begin(); iter != online.end(); ++iter)
            {
                if(*iter == which)
                {
                    online.erase(iter);
                    offline.push_back(which);
                    break;
                }
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {

        }
        void ShowMachines()
        {
            mtx.lock();
            std::cout<<"当前在线主机列表：";
            for(auto &id : online)
            {
                std::cout << id <<" ";
            }
            std::cout<<std::endl;
            for(auto &id : offline)
            {
             std::cout<<"当前离线主机列表：";
                std::cout << id << " ";
            }
            std::cout<<std::endl;
            mtx.unlock();
        }
    };


    class Control
    {
    private:
        ns_model::Model _model; //提供后台服务
        ns_view::View _view;    //提供html渲染功能
        LoadBlance _load_blance; //提供负载均衡器
    public:
        Control()
        {

        }
        //根据题目数据构建网页
        bool ALlQuestions(std::string *html)
        {
            std::vector<ns_model::Question> all;
            if(_model.GetAllQuestions(&all))
            {
                //获取题目信息成功，将所有的题目数据构建成网页
                _view.AllExpandHtml(all,html);
            }
            else
            {
                *html = "获取题目失败，形成题目列表失败";
                return false;
            }
            return true;
        }
        bool Question(const std::string number,std::string *html)
        {
            ns_model::Question q;
            if(_model.GetOneQuestion(number,&q))
            {
                //获取指定题目成功，将题目数据构建成网页
                _view.OneExpandHtml(q,html);
            }
            else
            {
                *html = "指定题目" + number + "不存在";
                return false;
            }
            return true;
        }
        void Judge(const std::string& number, const std::string in_json,std::string *out_json)
        {
            //0.根据题号，直接拿到题目细节
            ns_model::Question q;
            _model.GetOneQuestion(number,&q);
            //1.in_json进行反序列化，得到题目的id，得到用户提交的源代码，input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json,in_value);
            //2.重新拼接用户代码 + 测试用例代码，形成新代码
            std::string code = in_value["code"].asString();
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            Json::FastWriter writer;
            std::string complie_string = writer.write(compile_value);
            //3.选择负载最低的主机（差错处理）
            for( ; ;)
            {
                int id = 0;
                Machine *m = nullptr;
                if(!_load_blance.SmartChoice(&id,&m))
                {
                    break;
                }
                ns_log::LOG(ns_log::INFO) <<"选择主机成功"<<id<<"详情"<<m->ip<<":"<<m->port<<"\n";
                //4.然后发起http请求，得到结果
                httplib::Client cli(m->ip,m->port);
                m->IncLoad();
                if(auto res = cli.Post("/compile_and_run",complie_string, "application/json;charset=utf-8"))
                {
                    //5.将结果赋值给out_json
                    if(res->status == 200)
                    {
                         *out_json = res->body;
                         m->DecLoad();
                         break;
                    }
                     m->DecLoad();
                }
                else
                {
                    //请求失败
                    ns_log::LOG(ns_log::ERROR)<<"详情：" << id <<":"<< m->ip << ":"<<m->port<<"可能已经离线"<<"\n";
                    _load_blance.OfflineMachine(id);
                    _load_blance.ShowMachines();
                }   
            }  
        }
        ~Control()
        {

        }
    };
}