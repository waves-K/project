// model?
// MVC(经典的软件设计方式. 20年前就有了，现在仍然广泛使用)
// 现代还用一些更先进的方式比如 MVVM
// M => model: 负责数据存储
// V => view: 负责显示界面
// C => controller: 核心业务逻辑

// 基于文件的方式完成题目的存储
// 约定每一个题目对应一个目录，目录的名字就是题目的 id
// 目录里面包含以下几个文件：
//    1) header.cpp 代码框架
//    2) tail.cpp 代码测试用例
//    3) desc.txt 题目详细描述
// 除此之外，在搞一个 oj_config.cfg 文件，作为一个总的入口文件.
// 这个文件是一个行文本文件.
// 这个文件的每一行对应一个需要被服务器加载起来的题目
// 这一行里面包含以下几个信息：
//     a) 题目的 id 
//     b) 题目的名字
//     c) 题目的难度
//     d) 题目对应的目录
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include "util.hpp"
// model 这个模块要做的事情就是把刚才在文件中存储的题目信息加载起来.
// 供服务器随时使用

struct Question {
    std::string id;
    std::string name;
    std::string dir;  // 题目对应的目录. 目录就包含了题目描述/题目的代码框架/题目的测试用例
    std::string star;  // 表示难度

    // 以下这几个字段要根据 dir 字段来获取到
    std::string desc;  // 题目的描述
    std::string header_cpp;  // 题目的代码框架中的代码
    std::string tail_cpp;    // 题目的测试用例代码
};

// TODO 完善 OjModel 类，实现以下的基本操作
class OjModel {
    private:
        // 专门用来算字符串哈希值的两种算法：
        // MD5, SHA1 的特点
        // 1. 计算的哈希值非常均匀(两个字符串哪怕只差一个字符，计算得到的哈希值，也会差别很大)
        // 2. 不可逆(通过字符串算 hash 值很容易，但是给你 hash 值找到对应的原串，这件事理论上不可能)
        // 3. 固定长度(不管字符串多长，得到的哈希值都是固定长度)
        std::map<std::string, Question> model_;
    public:
        // 把文件上的数据加载到内存中, 加到 哈希表 中
        bool Load() {
            // 1. 先打开 oj_confic.cfg 文件
            std::ifstream file("./oj_data/oj_config.cfg");
            if (!file.is_open()) {
                return false;
            }
            // 2. 按行读取 oj_config.cfg 文件，并且解析
            std::string line;
            while (std::getline(file, line)) {
                // 3. 根据解析结果拼装成 Question 结构体
                std::vector<std::string> tokens;
                StringUtil::Split(line, "\t", &tokens);
                if (tokens.size() != 4) {
                    LOG(ERROR) << "config file format error!" << std::endl;
                    continue;
                }
                // 4. 把结构体加入到 hash 表中
                Question q;
                q.id = tokens[0];
                q.name = tokens[1];
                q.star = tokens[2];
                q.dir = tokens[3];
                FileUtil::Read(q.dir + "/desc.txt", &q.desc);
                FileUtil::Read(q.dir + "/header.cpp", &q.header_cpp);
                FileUtil::Read(q.dir + "/tail.cpp", &q.tail_cpp);
                // [] 如果 key 不存在，就创建新的键值对；如果 key 存在，就查找到对应的 value
                model_[q.id] = q;
            }
            file.close();
            LOG(INFO) << "Load " << model_.size() << " questions" << std::endl;
            return true;
        }

        bool GetAllQuestions(std::vector<Question>* questions) const {
            // 遍历哈希表就行了.
            // C++ 里面学过的很多的性能优化手段，都是在挠痒痒
            // auto 推导出来的类型是一个键值对，
            // push_back 需要的类型是 Question
            for (const auto& kv : model_) {
                questions->push_back(kv.second);
            }
            return true;
        }

        bool GetQuestion(const std::string& id, Question* q) const {
            std::map<std::string, Question>::const_iterator pos
                = model_.find(id);
            if (pos == model_.end()) {
                // 该 id 未查找到
                return false;
            }
            *q = pos->second;
            return true;
        }

};
