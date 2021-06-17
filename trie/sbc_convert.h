/** @file sbc_convert.h
  * @brief 全角转半角
  * Thinks https://github.com/andyzty/sensitivewd-filter
  * @author teng.qing
  * @date 2021/6/16
  */

#ifndef INC_01_TRIE_TREE_SBC_CONVERT_H_
#define INC_01_TRIE_TREE_SBC_CONVERT_H_

#include <string>

/** @class sbc_convert
  * @brief
  */
class SBCConvert {
public:
    /** @fn s2ws
      * @brief utf8 string convert to wstring
      * @param [in]str: utf8 string
      * @return utf8 wstring
      */
    static std::wstring s2ws(const std::string &str);

    /** @fn ws2s
      * @brief utf8 wstring convert to string
      * @param [in]str: utf8 wstring
      * @return utf8 string
      */
    static std::string ws2s(const std::wstring &w_str);

public:
    /** @fn ws_qj2bj
      * @brief 全角字符转换为半角字符，只处理全角的空格，全角！到全角～之间的字符，忽略其他
      * @param [in]src: 全角字符
      * @return 半角字符
      */
    static std::wstring ws_qj2bj(const std::wstring &src);

    /** @fn charConvert
      * @brief 大写转化为小写 全角转化为半角
      * @param [in]src: utf8字符
      * @return unicode编码
      */
    static int charConvert(const wchar_t &src);

    /** @fn qj2bj
      * @brief brief
      * @param src：4个字节
      * @return unicode编码
      */
    static int qj2bj(const wchar_t &src);
};

#ifdef UNIT_TEST

void test_ws_qj2bj();

void test_qj2bj();

void test_charConvert();

#endif

#endif //INC_01_TRIE_TREE_SBC_CONVERT_H_
