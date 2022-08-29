#include "complie_run.hpp"
#include "../comm/httplib.h"
using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
  std::cerr << "Usage: "
            << "\n\t" << proc << " port" << std::endl;
}

//编译服务随时可能被多个人请求，必须保证传递上来的code，形成源文件名称的时候，要具有
//唯一性，要不然多个用户之间会互相影响
//./compile_server port
int main(int argc, char *argv[])
{
  //通过http 让client给我们上传一个json_string

  //测试Start功能
  // std::string in_json;
  // Json::Value in_value;
  // in_value["code"] = R"(#include<iostream>
  //                       int main()
  //                       {
  //                         while(1);
  //                         std::cout<<"hello world"<<std::endl;
  //                       })";
  // in_value["input"] = "";
  // in_value["cpu_limit"] = 1;
  // in_value["mem_limit"] = 10240 * 3;
  // Json::FastWriter writer;
  // in_json = writer.write(in_value);
  // std::cout<<in_json<<std::endl;

  // std::string out_json;
  // ns_compile_and_run::ComplieAndRun::Start(in_json,&out_json);
  // std::cout<<out_json<<std::endl;
  // return 0;
  if (argc != 2)
  {
    Usage(argv[0]);
    return 1;
  }
  Server svr;
  svr.Post("/compile_and_run", [](const Request &req, Response &resp)
           {
        // 用户请求的服务正文是我们想要的json string
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty()){
            ComplieAndRun::Start(in_json, &out_json);
            resp.set_content(out_json, "application/json;charset=utf-8");
        } });

  // svr.set_base_dir("./wwwroot");
  svr.listen("0.0.0.0", atoi(argv[1])); //启动http服务
}