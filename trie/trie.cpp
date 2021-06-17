/** @file trie.h
  * @brief trie树算法实现的敏感词过滤
  *
  * Trie 树原理及其敏感词过滤的实现: https://www.jianshu.com/p/9919244dd7ad
  * wildfirechat: https://github.com/wildfirechat/server/blob/wildfirechat/broker/src/main/java/win/liyufan/im/SensitiveFilter.java
  * trie: https://github.com/r-lyeh-archived/trie/blob/master/trie.hpp
  *
  * Thinks [sensitivewd-filter](https://github.com/andyzty/sensitivewd-filter)
  *
  * @author teng.qing
  * @date 2021/6/10
  */

#include "trie.h"
#include <iostream>
#include <fstream>

const wchar_t kEndFlag = L'ﾰ'; // unicode: FFB0

TrieNode::TrieNode() = default;

TrieNode::~TrieNode() {
    for (auto i : subNodes_) {
        // std::cout << "delete" << i.first << std::endl;
        delete i.second;
    }
    subNodes_.clear();
}

Trie::Trie() { root_ = new TrieNode(); }

Trie::~Trie() {
    delete root_;
    root_ = nullptr;
}

void Trie::insert(const std::wstring &word) {
    TrieNode *curNode = root_;
    for (wchar_t code : word) {
        int unicode = SBCConvert::charConvert(code);
        TrieNode *subNode = curNode->getSubNode(unicode);

        // 如果没有这个节点则新建
        if (subNode == nullptr) {
            subNode = new TrieNode();
            curNode->addSubNode(unicode, subNode);
        }
        // 指向子节点，进入下一循环
        curNode = subNode;
    }
    // 设置结束标识
    int unicode = SBCConvert::charConvert(kEndFlag);
    curNode->addSubNode(unicode, new TrieNode());
}

bool Trie::search(const std::wstring &word) {
    bool is_contain = false;
    for (int p2 = 0; p2 < word.length(); ++p2) {
        int wordLen = getSensitiveLength(word, p2);
        if (wordLen > 0) {
            is_contain = true;
            break;
        }
    }
    return is_contain;
}

bool Trie::startsWith(const std::wstring &prefix) {
    TrieNode *curNode = root_;
    for (wchar_t item : prefix) {
        int unicode = SBCConvert::charConvert(item);
        curNode = curNode->getSubNode(unicode);
        if (curNode == nullptr)
            return false;
    }
    return true;
}

std::set<SensitiveWord> Trie::getSensitive(const std::wstring &word) {
    std::set<SensitiveWord> sensitiveSet;

    for (int p2 = 0; p2 < word.length(); ++p2) {
        int wordLen = getSensitiveLength(word, p2);
        if (wordLen > 0) {
            std::wstring sensitiveWord = word.substr(p2, wordLen);
            SensitiveWord wordObj;
            wordObj.word = sensitiveWord;
            wordObj.startIndex = p2;
            wordObj.len = wordLen;

            sensitiveSet.insert(wordObj);
            p2 = p2 + wordLen - 1;
        }
    }

    return sensitiveSet;
}

int Trie::getSensitiveLength(std::wstring word, int startIndex) {
    TrieNode *p1 = root_;
    int wordLen = 0;
    bool endFlag = false;

    for (int p3 = startIndex; p3 < word.length(); ++p3) {
        int unicode = SBCConvert::charConvert(word[p3]);
        auto subNode = p1->getSubNode(unicode);
        if (subNode == nullptr) {
            // 如果是停顿词，直接往下继续查找
            if (stop_words_.find(unicode) != stop_words_.end()) {
                ++wordLen;
                continue;
            }

            break;
        } else {
            ++wordLen;
            // 直到找到尾巴的位置，才认为完整包含敏感词
            if (subNode->getSubNode(SBCConvert::charConvert(kEndFlag))) {
                endFlag = true;
                break;
            } else {
                p1 = subNode;
            }
        }
    }

    // 注意，处理一下没找到尾巴的情况
    if (!endFlag) {
        wordLen = 0;
    }
    return wordLen;
}

#if 0
/** @fn
  * @brief linux下一个中文占用三个字节,windows占两个字节
  * 参考：https://blog.csdn.net/taiyang1987912/article/details/49736349
  * @param [in]str: 字符串
  * @return
  */
std::string chinese_or_english_append(const std::string &str) {
    std::string replacement;
    //char chinese[4] = {0};
    int chinese_len = 0;
    for (int i = 0; i < str.length(); i++) {
        unsigned char chr = str[i];
        int ret = chr & 0x80;
        if (ret != 0) { // chinese: the top is 1
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD) {
                    chinese_len = 6;
                } else if (chr >= 0xF8) {
                    chinese_len = 5;
                } else if (chr >= 0xF0) {
                    chinese_len = 4;
                } else if (chr >= 0xE0) {
                    chinese_len = 3;
                } else if (chr >= 0xC0) {
                    chinese_len = 2;
                } else {
                    throw std::exception();
                }
            }
            // 跳过
            i += chinese_len - 1;

            //chinese[0] = str[i];
            //chinese[1] = str[i + 1];
            //chinese[2] = str[i + 2];
        } else /** ascii **/ {
        }
        replacement.append("*");
    }
    return replacement;
}
#endif

std::wstring Trie::replaceSensitive(const std::wstring &word) {
    std::set<SensitiveWord> words = getSensitive(word);
    std::wstring ret = word;
    for (auto &item : words) {
        for (int i = item.startIndex; i < (item.startIndex + item.len); ++i) {
            ret[i] = L'*';
        }
    }
    return ret;
}

void Trie::loadFromFile(const std::string &file_name) {
    std::ifstream ifs(file_name, std::ios_base::in);
    std::string str;
    int count = 0;
    while (getline(ifs, str)) {
        std::wstring utf8_str = SBCConvert::s2ws(str);
        insert(utf8_str);
        count++;
    }
    std::cout << "load " << count << " words" << std::endl;
}

void Trie::loadStopWordFromFile(const std::string &file_name) {
    std::ifstream ifs(file_name, std::ios_base::in);
    std::string str;
    int count = 0;
    while (getline(ifs, str)) {
        std::wstring utf8_str = SBCConvert::s2ws(str);
        if (utf8_str.length() == 1) {
            stop_words_.insert(utf8_str[0]);
            count++;
        } else if (utf8_str.empty()) {
            stop_words_.insert(L' ');
            count++;
        }
    }
    std::cout << "load " << count << " stop words" << std::endl;
}

void Trie::loadStopWordFromMemory(std::unordered_set<wchar_t> &words) {
    for (auto &str : words) {
        int unicode = SBCConvert::charConvert(str);
        stop_words_.emplace(unicode);
    }
}

void Trie::loadFromMemory(std::unordered_set<std::wstring> &words) {
    for (auto &item : words) {
        insert(item);
    }
}

#ifdef UNIT_TEST

#include <thread>

// performance
void test_time(Trie &t) {
    auto t1 = std::chrono::steady_clock::now();

    std::cout << "你好，加一下微信18301231231:";
    for (auto &&i : t.getSensitive(L"你好，加一下微信VX18301231231")) {
        std::cout << SBCConvert::ws2s(i.word);
    }
    std::cout << std::endl;

    // run code
    auto t2 = std::chrono::steady_clock::now();
    //毫秒级
    double dr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "耗时: " << dr_ms << std::endl;
}

void func(Trie &t) {
    std::cout << "敏感词为：";
    std::wstring origin = L"你你你你是傻逼啊你，说你呢，你个大笨蛋。";
    wchar_t dest[128] = {0};
    origin.copy(dest, origin.length(), 0);

    for (auto &&i : t.getSensitive(origin)) {
        std::cout << "[index=" << i.startIndex << ",len=" << i.len
                  << ",word=" << SBCConvert::ws2s(i.word) << "],";

        // PS：对于中文敏感词，因为一般汉字占3个字节（也有4个字节），所以这里“*”是正常的3倍。
        // 项目里需要是判断是包含，而不是替换。
        // 所以，如果有这种需求，建议改成wstring和wchat_t来实现
        for (int j = i.startIndex; j < (i.startIndex + i.len); ++j) {
            dest[j] = '*';
        }
    }

    std::cout << "  替换后：" << dest << std::endl;
    std::cout << std::endl;
}

// thread safe
void test_concurrent(Trie &t) {
    for (int i = 0; i < 20; i++) {
        std::thread thd([&]() { func(t); });
        thd.join();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(3000));
}

int testTrie() {
    Trie t;
    t.insert(L"apple");
    assert(t.search(L"apple"));            // 返回 true
    assert(t.search(L"app") == false);     // 返回 false
    assert(t.startsWith(L"app"));          // 返回 true
    t.insert(L"app");

    assert(t.search(L"app"));            // 返回 true
    assert(t.search(L"this is apple"));  // 返回 true

    t.insert(L"微信");
    t.insert(L"vx");
    std::wstring origin = L"你好，请加微信183023102312";
    assert(t.replaceSensitive(origin) == L"你好，请加**183023102312");

    t.insert(L"你是傻逼");
    t.insert(L"你是傻逼啊");
    t.insert(L"你是坏蛋");
    t.insert(L"你个大笨蛋");
    t.insert(L"我去年买了个表");
    t.insert(L"shit");

    origin = L"SHit，你你你你是傻逼啊你，说你呢，你个大笨蛋。";
    assert(t.replaceSensitive(origin) == L"****，你你你****啊你，说你呢，*****。");

    test_time(t);
    test_concurrent(t);

    // 测试汉字的字节数
    // std::string str = "你好啊";
    // std::string test = std::to_wstring(str);

    // for (int i = 0; i < test.length(); i++) {
    //   std::cout << test[i] << std::endl;
    // }

    return 0;
}

#endif // UNIT_TEST