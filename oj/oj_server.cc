#include "httplib.h"
#include <jsoncpp/json/json.h>

#include "util.hpp"
#include "oj_model.hpp"
#include "oj_view.hpp"
#include "compile.hpp"

// controller 作为服务器核心逻辑，需要创建好对应的服务器框架代码
// 在这个框架中来组织逻辑

int main() {
    // 服务器启动只加载一次数据即可
    OjModel model;
    model.Load();

    using namespace httplib;
    Server server;
    server.Get("/all_questions", [&model](const Request& req,
                Response& resp) {
            (void) req;
            // 数据来自于 Model 对象
            std::vector<Question> all_questions;
            model.GetAllQuestions(&all_questions);
            // 如何借助 all_questions 数据得到最终的 HTML?
            std::string html;
            OjView::RenderAllQuestions(all_questions, &html);
            resp.set_content(html, "text/html");
            });

    // R"()" C++ 11 引入的语法，原始字符串(忽略字符串中的转义字符)
    // \d+ 正则表达式，用一些特殊符号来表示字符串具有满足啥样的条件
    server.Get(R"(/question/(\d+))", [&model](const Request& req, Response& resp) {
            // LOG(INFO) << req.matches[0].str() << ", "
            // << req.matches[1].str() << std::endl;
            Question question;
            model.GetQuestion(req.matches[1].str(), & question);
            LOG(INFO) << "desc:" << question.desc << std::endl;
            std::string html;
            OjView::RenderQuestion(question, &html);
            resp.set_content(html, "text/html");
            });

    server.Post(R"(/compile/(\d+))", [&model](const Request& req, Response& resp) {
            // 1. 现根据 id 获取到题目信息
            Question question;
            model.GetQuestion(req.matches[1].str(), &question);

            // 2. 解析 body , 获取到用户提交的代码
            std::unordered_map<std::string, std::string> body_kv;
            UrlUtil::ParseBody(req.body, &body_kv);
            const std::string& user_code = body_kv["code"];

            // 3. 构造 JSON 结构的参数
            Json::Value req_json; 
            // 真实需要编译的代码，是用户提交的代码 + 题目的测试用例代码
            req_json["code"] = user_code + question.tail_cpp;
            Json::Value resp_json;  // resp_json 放到响应中
            // 4. 调用编译模块进行编译
            Compiler::CompileAndRun(req_json, &resp_json);

            // 5. 根据编译结果构造成最终的网页
            std::string html;
            OjView::RenderResult(resp_json["stdout"].asString(),
                    resp_json["reason"].asString(), &html);
            resp.set_content(html, "text/html");
            });
    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0", 9092);
    return 0;
}
