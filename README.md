# cpp-dirtyfilter

C++中文敏感词过滤，支持特殊符号，半角全角，停顿词，重复词等等，基于trie树算法实现

# Feature

- [x] trie算法
- [x] 中文敏感词
- [x] 英文敏感词
    - [x] 大小写
    - [x] 全角字母
- [x] 停顿词
    - [x] 半角停顿词(ASCII)
    - [x] 全角停顿词(除ASCII以外，中文标点、Emotion表情等)
- [x] 重复词
    - [x] 半角停顿词(ASCII)
    - [x] 全角

# Usage

## Base

```c++
#include "trie.h"

int main() {
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
}
```

output:
```bash
****，你是逗比吗？****，你竟然用**，*********
```

## Examples

```c++
int main() {
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
```

```bash
load 653 words
load 46 stop words
[cost: 0.011371 ms]FUCK => ****
[cost: 0.003244 ms]FuCk => ****
[cost: 0.007321 ms]F&uc&k => ******
[cost: 0.005006 ms]F&uc&&&k => ********
[cost: 0.003155 ms]ＦＵｃｋ => ****
[cost: 0.030613 ms]F。uc——k => F。uc——k
[cost: 0.006558 ms]fＵｃk => ****
[cost: 0.006181 ms]fＵ😊ｃk => *****
[cost: 0.00511 ms]微信 => **
[cost: 0.006742 ms]微——信 => 微——信
[cost: 0.012082 ms]微【】、。？《》信 => *********
[cost: 0.004329 ms]微。信 => ***
[cost: 0.004665 ms]VX => **
[cost: 0.003428 ms]vx => **
[cost: 0.005998 ms]V&X => ***
[cost: 0.031304 ms]微!., #$%&*()|?/@"';[]{}+~-_=^<>信 => ********************************
[cost: 0.004827 ms]扣扣 => **
[cost: 0.00585 ms]扣_扣 => ***
[cost: 0.00435 ms]QQ => **
[cost: 0.00346 ms]Qq => **
```

# build

```bash
$ git clone https://github.com/xmcy0011/cpp-dirtyfilter
$ cd cpp-dirtyfilter/trie
$ mkdir build && cd build
$ cmake ..
$ cp ../../stopwd.txt .
$ cp ../../word.txt .
$ ./trie
```

# tire数算法详解

- [IM敏感词算法原理和实现](https://blog.csdn.net/xmcy001122/article/details/118000803)

# 词库

- [中文敏感词库](https://www.heywhale.com/mw/dataset/5ecf6214162df90036ddfcff/content)

- [mgck](https://github.com/chason777777/mgck)

- [textfilter：敏感词过滤的几种实现+某1w词敏感词库](https://github.com/observerss/textfilter)

- [sensitive-stop-words：互联网常用敏感词、停止词词库](https://github.com/fwwdn/sensitive-stop-words)

- [keywordfilter：5W多个违法词](https://github.com/k5h9999/keywordfilter)

# Contact

email: xmcy0011@sina.com
blog: https://blog.csdn.net/xmcy001122