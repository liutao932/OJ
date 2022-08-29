#include <iostream>
#include <string>
#include <ctemplate/template.h>

int main()
{
    std::string in_html = "./test.html";
    std::string value = "成都东软学院";

    // 形成数据字典
    ctemplate::TemplateDictionary root("test"); //unordered_map<> test;
    root.SetValue("key", value);                //test.insert({});

    // 获取被渲染网页对象
    ctemplate::Template *tpl = ctemplate::Template::GetTemplate(in_html, ctemplate::DO_NOT_STRIP);

    // 添加字典数据到网页中
    std::string out_html;
    tpl->Expand(&out_html, &root);

    //完成了渲染
    std::cout << out_html << std::endl;
}