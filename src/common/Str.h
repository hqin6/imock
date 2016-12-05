/**
 * 文件名称：Str.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _STR_H_
#define _STR_H_

#include <string>
#include "EList.h"

enum SPLIT_MODE
{
    SINGLE_SPLIT,
    WHOLE_SPLIT
};

class Str
{
    public:
        static void Replace(std::string& src, 
                const std::string& from, 
                const std::string& to);
        static void RTrim(std::string& str, 
                const std::string& tc = "");
        static void LTrim(std::string& str, 
                const std::string& tc = "");
        static void Trim(std::string& str, 
                const std::string& tc = "");
        static void SplitTrimNoNull(
                const std::string& str, 
                EList<std::string>& arr,
                const std::string& dmt = "\t",
                SPLIT_MODE sm = WHOLE_SPLIT);
        static void SplitEscaped(
                const std::string& str, 
                EList<std::string>& arr);
        static bool Tok(const std::string& src, int& bi,
                std::string& tok, int fixLen);
        static bool Tok(const std::string& src, int& bi,
                std::string& tok, const char* split = NULL);
        static std::string UrlEnc(const std::string& data);
        static std::string UrlDec(const std::string& data);
        static std::string ToPrint(const std::string& data);
        static std::string ToPrintXml(const std::string& data);
        static std::string Esc(const std::string& data);
        static std::string B64Enc(const std::string& data);
        static std::string B64EncNoAppend(const std::string& data);
        static std::string SafeB64Enc(const std::string& data);
        static std::string SafeB64EncNoAppend(const std::string& data);
        static std::string B64Dec(const std::string& data);
        static std::string B64DecNoAppend(const std::string& data);
        static std::string SafeB64Dec(const std::string& data);
        static std::string SafeB64DecNoAppend(const std::string& data);
        static bool AtoI(const std::string& s, int& r);
        static bool AtoF(const std::string& s, double& r);
        static std::string ToKMGT(double v);
};

#endif
