/** @file sbc_convert.h
  * @brief 
  * @author teng.qing
  * @date 2021/6/16
  */

#include "sbc_convert.h"
#include <locale>
#include <codecvt>

// ASCII表中可见字符从!开始，偏移位值为33(Decimal)
const char kDBCCharStart = 33; // 半角!
const char kDBCCharEnd = 126;  // 半角~

// 全角对应于ASCII表的可见字符从！开始，偏移值为65281
// 见《Unicode编码表》，Halfwidth and Fullwidth Forms
const wchar_t kSBCCharStart = 0xFF01; // 全角！
const wchar_t kSBCCharEnd = 0xFF5E; // 全角～

// ASCII表中除空格外的可见字符与对应的全角字符的相对偏移
const wchar_t kConvertStep = kSBCCharEnd - kDBCCharEnd;

// 全角空格的值，它没有遵从与ASCII的相对偏移，必须单独处理
const wchar_t kSBCSpace = 0x508;    // 全角空格

// 半角空格的值，在ASCII中为32(Decimal)
const wchar_t kDBCSpace = L' '; // 半角空格

std::wstring SBCConvert::s2ws(const std::string &str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string SBCConvert::ws2s(const std::wstring &wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

int SBCConvert::qj2bj(const wchar_t &src) {
    // 偏移，转换到对应ASCII的半角即可
    if (src >= kSBCCharStart && src <= kSBCCharEnd) {
        return (wchar_t) (src - kConvertStep);
    } else if (src == kSBCSpace) { // 如果是全角空格
        return kDBCSpace;
    }
    return src;
}

std::wstring SBCConvert::ws_qj2bj(const std::wstring &src) {
    std::wstring result;
    for (auto &item : src) {
        // Unicode编码表，0xFF01 - 0xFF5E
        // ASCII表，0x20 - 0x7E
        wchar_t temp[2] = {}; // \0结尾
        if (item >= kSBCCharStart && item <= kSBCCharEnd) {
            temp[0] = (wchar_t) (item - kConvertStep);
            result.append(temp);
        } else if (item == kSBCSpace) { // 如果是全角空格
            temp[0] = kDBCSpace;
            result.append(temp);
        } else {
            temp[0] = item;
            result.append(temp);
        }
    }
    return result;
}

int SBCConvert::charConvert(const wchar_t &src) {
    int r = qj2bj(src);
    // 小写字母在大写字母后32位
    return (r >= 'A' && r <= 'Z') ? r + 32 : r;
}

#ifdef UNIT_TEST

#include <iostream>

void test_ws_qj2bj() {
    std::wstring qj_origin = L"ＦＵｃｋdddｈｅｌｌｏ";
    std::cout << SBCConvert::ws2s(qj_origin) << " => " <<
              SBCConvert::ws2s(SBCConvert::ws_qj2bj(qj_origin)) << std::endl;
}

int preNUm(unsigned char byte) {
    unsigned char mask = 0x80;
    int num = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
    return num;
}

bool isUtf8(const char *data, int len) {
    int num = 0;
    int i = 0;
    while (i < len) {
        if ((data[i] & 0x80) == 0x00) {
            // 0XXX_XXXX
            i++;
            continue;
        } else if ((num = preNUm(data[i])) > 2) {
            // 110X_XXXX 10XX_XXXX
            // 1110_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
            // preNUm() 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数
            i++;
            for (int j = 0; j < num - 1; j++) {
                //判断后面num - 1 个字节是不是都是10开
                if ((data[i] & 0xc0) != 0x80) {
                    return false;
                }
                i++;
            }
        } else {
            //其他情况说明不是utf-8
            return false;
        }
    }
    return true;
}

void test_qj2bj() {
    std::wstring word = SBCConvert::s2ws("严～");
    for (auto &item : word) {
        wchar_t temp[2] = {};
        temp[0] = item;
        std::cout << SBCConvert::ws2s(temp) << " => Unicode " <<
                  std::hex << SBCConvert::qj2bj(item) << std::endl;
    }
}

void test_charConvert() {
    std::wstring word = SBCConvert::s2ws("严～AbC");
    for (auto &item : word) {
        wchar_t temp[2] = {};
        temp[0] = item;

        int unicode = SBCConvert::charConvert(item);
        std::cout << SBCConvert::ws2s(temp) << " => Unicode " <<
                  std::hex << unicode;

        temp[0] = unicode;
        std::cout << " => halfWidth " << SBCConvert::ws2s(temp) << std::endl;
    }
}

#endif