/** @file main.cpp
  * @brief main
  * @author teng.qing
  * @date 2021/6/17
  */

#include "trie.h"
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <fstream>

int example1() {
    std::unordered_set<std::wstring> sensitive = {
            L"fuck",
            L"微信",
    };
    std::unordered_set<wchar_t> stop_words = {
            L'@',
            L'!',
            L'%',
            L'。',
    };
    Trie trie;
    trie.loadFromMemory(sensitive);
    trie.loadStopWordFromMemory(stop_words);

    std::string str = "fUcK，你是逗比吗？ｆｕｃｋ，你竟然用微信，微@!!%%%。信";
    std::wstring result = trie.replaceSensitive(SBCConvert::s2ws(str));
    std::cout << SBCConvert::ws2s(result) << std::endl;
    return 0;
}

int exmaple2() {
    std::vector<std::string> words = {
            // 字母
            "FUCK",     // 全大写
            "FuCk",     // 混合
            "F&uc&k",   // 特殊符号
            "F&uc&&&k",  // 连续特殊符号
            "ＦＵｃｋ",   // 全角大小写混合
            "F。uc——k",  // 全角特殊符号
            "fＵｃk",    // 全角半角混合
            "fＵ😊ｃk",  // Emotion表情，测试

            // 简体中文
            "微信",
            "微——信",   // 全角符号
            "微【】、。？《》信", // 全角重复词
            "微。信",
            "VX",
            "vx", // 小写
            "V&X", // 特殊字符
            "微!., #$%&*()|?/@\"';[]{}+~-_=^<>信", // 30个特殊字符 递归
            "扣扣",
            "扣_扣",
            "QQ",
            "Qq",
    };

    Trie trie;
    trie.loadFromFile("word.txt");
    trie.loadStopWordFromFile("stopwd.txt");

    for (auto &item : words) {
        auto t1 = std::chrono::steady_clock::now();

        std::wstring result = trie.replaceSensitive(SBCConvert::s2ws(item));

        auto t2 = std::chrono::steady_clock::now();
        double dr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        std::cout << "[cost: " << dr_ms << " ms]" << item << " => " << SBCConvert::ws2s(result) << std::endl;
    }
    return 0;
}

double get_time_diff(std::chrono::steady_clock::time_point t1) {
    // run code
    auto t2 = std::chrono::steady_clock::now();
    // 毫秒级
    double dr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    return dr_ms;
}

void test_time(Trie &t, const std::wstring &origin_word) {
    auto t1 = std::chrono::steady_clock::now();
    std::cout << SBCConvert::ws2s(origin_word) << std::endl;

    auto r = t.getSensitive(origin_word);

    for (auto &&i : r) {
        std::cout << "[index=" << i.startIndex << ",len=" << i.len
                  << ",word=" << i.word.c_str() << "]," << std::endl;
    }

    // run code
    auto t2 = std::chrono::steady_clock::now();
    // 毫秒级
    double dr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "cost: " << dr_ms << " ms" << std::endl;

    std::cout << SBCConvert::ws2s(t.replaceSensitive(origin_word)) << std::endl << std::endl;
}

// thread safe
void test_concurrent(Trie &t, const std::wstring &origin_word) {
    for (int i = 0; i < 20; i++) {
        std::thread thd([&]() {
            std::cout << "thread: " << SBCConvert::ws2s(t.replaceSensitive(origin_word)) << std::endl;
        });
        thd.join();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(3000));
}

void test_time_by_find(std::vector<std::wstring> &words, std::wstring &origin_word) {
    std::cout << "find by std::string::find()" << std::endl;
    // 转换成小写
    transform(origin_word.begin(), origin_word.end(), origin_word.begin(), ::tolower);

    // 开始计时
    auto t1 = std::chrono::steady_clock::now();
    for (auto &&i : words) {
        size_t n = origin_word.find(i);
        if (n != std::string::npos) {
            std::cout << "[index=" << n << ",len=" << i.length() << ",word=" << SBCConvert::ws2s(i)
                      << "]," << std::endl;
        }
    }

    // run code
    auto t2 = std::chrono::steady_clock::now();
    //毫秒级
    double dr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "cost : " << dr_ms << " ms" << std::endl << std::endl;
}

void testStopWords() {
    Trie t;
    t.insert(L"vx");
    t.insert(L"微信");
    t.loadStopWordFromFile("stopwd.txt");

    std::wstring origin = L"请加微-信，v x 号为 12305234";
    auto r = t.getSensitive(origin);

    for (auto &&i : r) {
        std::cout << "[index=" << i.startIndex << ",len=" << i.len
                  << ",word=" << i.word.c_str() << "]," << std::endl;
    }

    std::cout << SBCConvert::ws2s(t.replaceSensitive(origin)) << std::endl;
}

int exmaple3() {
    // trie: small
    Trie trie2;
    trie2.insert(L"你是傻逼");
    trie2.insert(L"你是傻逼啊");
    trie2.insert(L"你是坏蛋");
    trie2.insert(L"你个大笨蛋");
    trie2.insert(L"我去年买了个表");
    trie2.insert(L"shit");
    trie2.insert(L"qq"); // 都会转换为小写
    trie2.insert(L"vx");

    std::wstring origin = SBCConvert::s2ws("SHit，QQ,VX 你你你你是傻逼啊你，说你呢，你个大笨蛋。");

    test_time(trie2, origin);

    // KMP: small
    std::vector<std::wstring> words;
    words.emplace_back(L"你是傻逼");
    words.emplace_back(L"你是傻逼啊");
    words.emplace_back(L"你是坏蛋");
    words.emplace_back(L"你个大笨蛋");
    words.emplace_back(L"我去年买了个表");
    words.emplace_back(L"shit");
    test_time_by_find(words, origin);

    // trie: large from file
    Trie t;
    t.loadFromFile("word.txt");

    origin = L"你个傻逼，小姐姐还不赶紧加VX，微信，扣扣是Qq3306 4343，你奶奶的。。。赶快加";

    auto t1 = std::chrono::steady_clock::now();
    std::cout << "origin: " << SBCConvert::ws2s(origin) << std::endl
              << "filter: " << SBCConvert::ws2s(t.replaceSensitive(origin)) << std::endl
              << "cost: " << get_time_diff(t1) << " ms" << std::endl << std::endl;

    // KMP: large from file
    std::ifstream ifs("word.txt", std::ios_base::in);
    std::string str;
    words.clear(); // clear
    while (getline(ifs, str)) {
        words.emplace_back(SBCConvert::s2ws(str));
    }
    test_time_by_find(words, origin);

    // thread safe
    test_concurrent(trie2, origin);

    // stop words
    std::cout << "test stop words" << std::endl;
    testStopWords();
    return 0;
}

int main() {
    example1();
    exmaple2();
    //exmaple3();
    return 0;
}