#include <iostream>
#include "../comm/httplib.h"
#include "oj_control.hpp"
using namespace httplib;

int main()
{
    //用户请求的的路由功能
    Server svr;
    ns_control::Control ctrl;
    //获取所有题目列表
    svr.Get("/all_questions",[&ctrl](const Request& req,Response &resq){
        //返回一张带有所有题目的html网页
        std::string html;
        ctrl.ALlQuestions(&html);
        resq.set_content(html,"text/html;charset=utf-8");
    });
    //获取要根据题目编号，获取题目的内容
    svr.Get(R"(/question/(\d+))",[&ctrl](const Request& req,Response &resq){
        std::string num = req.matches[1];
        std::string html;
        ctrl.Question(num,&html);
        resq.set_content(html,"text/html;charset=utf-8");
    });
    //用户提交代码，使用我们的判题功能()
    svr.Post(R"(/judge/(\d+))",[&ctrl](const Request& req,Response &resq){
        std::string number = req.matches[1];
        std::string result_json;
        ctrl.Judge(number,req.body,&result_json);
        resq.set_content(result_json,"application/json;charset=utf-8");
       // resq.set_content("指明题目的判题:"+num,"text/plain;charset=utf-8");
    });

    
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0",8080);
    return 0;
}