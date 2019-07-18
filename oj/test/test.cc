#include <jsoncpp/json/json.h>
#include "../util.hpp"
#include "../compile.hpp"

int main() {
    Json::Value req_json;
    req_json["code"] = "#include <stdio.h>\n int main() {printf(\"hehe\");return 0;}";
    req_json["stdin"] = "";
    Json::Value resp_json;
    Compiler::CompileAndRun(req_json, &resp_json);
    Json::FastWriter writer;
    LOG(INFO) << writer.write(resp_json) << std::endl;
    return 0;
}
