#include "Str.h"
#include <stdio.h>
#include<boost/tokenizer.hpp>
using namespace std;
using namespace boost;

void Str::Replace(string& src, const string& from, const string& to)
{
    string::size_type pos = 0;
    while ( (pos = src.find(from, pos)) != string::npos)
    {
        src.replace(pos, from.length(), to);
        pos += to.length();

    }
}

void Str::LTrim(string& str, const string& tc)
{
    string::size_type p = str.find_first_not_of(tc + " \t\r\n");
    if (string::npos == p)
    {
        str = "";
    }
    str = str.substr(p);
}

void Str::RTrim(string& str, const string& tc)
{
    string::size_type q = str.find_last_not_of(tc + " \t\r\n");
    str = str.substr(0, q + 1);
    return;
}

void Str::Trim(string& str, const string& tc)
{
    string::size_type p = str.find_first_not_of(tc + " \t\r\n");
    if (string::npos == p)
    {
        str = "";
        return;
    }
    string::size_type q = str.find_last_not_of(tc + " \t\r\n");
    str = str.substr(p, q - p + 1);
    return;
}

void Str::SplitTrimNoNull(const string& src, 
        EList<string>& arr, const string& dmt, SPLIT_MODE sm)
{
    string::size_type p = 0;
    string::size_type q;
    arr.clear();
    while (true)
    {
        if (SINGLE_SPLIT == sm)
        {
            q = src.find_first_of(dmt, p);
        }
        else
        {
            q = src.find(dmt, p);
        }
        string str = src.substr(p, q - p);
        Trim(str);
        if (! str.empty()) 
        {
            arr.push_back(str);
        }
        if (string::npos == q)
        {
            break;
        }
        if (SINGLE_SPLIT == sm)
        {
            p = q + 1;
        }
        else
        {
            p = q + dmt.size();
        }
    }
}

void Str::SplitEscaped(const string& str, EList<string>& arr)
{
  escaped_list_separator<char> es("\\", " \t", "\"");
  tokenizer<escaped_list_separator<char> > tok(str, es);
  for (tokenizer<escaped_list_separator<char> >::iterator 
          beg = tok.begin(); beg != tok.end(); ++beg)
  {
      string s = *beg;
      if (! s.empty())
      {
          arr.push_back(s);
      }
  }
}

bool Str::Tok(const string& src, int& bi, string& tok, int fixLen)
{
    if (-1 == bi)
    {//已经解析完了
        return false;
    }
    else if ((int)src.size() <= bi || bi == -2) 
    {//解析完了，本次解析出来的为空值
        bi = -1;
        tok = "";
        return true;
    }
    else if ((int)src.size() < bi + fixLen)
    {//解析长度不够
        return false;
    }
    else
    {
        tok = src.substr(bi, fixLen);
        bi += fixLen;
        if (bi >= (int)src.size())
        {
            bi = -2;
        }
        return true;
    }
}

bool Str::Tok(const string& src, int& bi, string& tok, const char* split)
{
    if (-1 == bi)
    {//已经解析完了
        return false;
    }
    else if ((int)src.size() <= bi || bi == -2) 
    {//解析完了，本次解析出来的为空值
        bi = -1;
        tok = "";
        return true;
    }
    else if (! split)
    {//如果没有指定split，则将所有的元素取出
        tok = src.substr(bi);
        bi = -1;
        return true;
    }
    string sp = split;
    string::size_type idx;
    idx = src.find(sp, bi);
    if (string::npos == idx)
    {//未找到，则剩余所有值作为tok
        tok = src.substr(bi);
        bi = -1;
    }
    else
    {//找到了 
        tok = src.substr(bi, idx - bi);
        bi = idx + sp.size();
    }
    return true; 
}

static unsigned char hexchars[] = "0123456789ABCDEF";

string Str::UrlEnc(const string& data)
{
    register unsigned char c;
    unsigned char const *from, *end;

    from = (unsigned char *)data.c_str();
    end = (unsigned char *)from + data.size();
    string res;
    res.reserve(data.size() * 3);

    while (from < end) 
    {
        c = *from++;

        if (c == ' ') 
        {
            res += '+';
        } 
        else if ((c < '0' && c != '-' && c != '.') ||
                (c < 'A' && c > '9') ||
                (c > 'Z' && c < 'a' && c != '_') ||
                (c > 'z')) 
        {
            res += '%';
            res += hexchars[c >> 4];
            res += hexchars[c & 15];
        } 
        else 
        {
            res += c;
        }
    }
    return res;
}

static int Htoi(char *s)
{
    int value;
    int c;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
        c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
        c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value); 
}

string Str::UrlDec(const string& str)
{
    string res;
    res.reserve(str.size() * 3);

    char *data = const_cast<char*>(str.c_str());
    int len = str.size();

    while (len--) 
    {
        if (*data == '+') 
        {
            res += ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
                && isxdigit((int) *(data + 2))) 
        {
            res += (char) Htoi(data + 1);
            data += 2;
            len -= 2;
        } 
        else 
        {
            res += *data;
        }
        data++;
    }
    return res;
}

string Str::ToPrint(const string& data)
{
    const unsigned char* p = (unsigned char*)data.c_str();
    const unsigned char* e = p + data.size();
    string res = "";
    char buf[16];
    while (p != e)
    {
        if (isprint(*p))
        {
            res += *p;
        }
        else
        {
            res += "\\";
            snprintf(buf, sizeof(buf), "%o", *p);
            res += buf;
        }
        ++p;
    }
    return res;
}

#define IS_XML_FORBID(c) ((c)=='&' || (c)=='<' || (c)=='>' || (c)=='\'' || (c)=='"' || (c)==' ')
string Str::ToPrintXml(const string& data)
{
    // 判断是不是全为空
    bool allNull = true;
    for (int i = 0; i < (int)data.length(); ++i)
    {
#define IS_XML_WHITE_SPACE(c) ( isspace( (unsigned char) (c) ) || (c) == '\n' || (c) == '\r')
        if (! IS_XML_WHITE_SPACE(data[i]))
        {
            allNull = false;
            break;
        }
    }
    if (allNull && data.length() > 0)
    {
        return "<![CDATA[" + data +  "]]> ";
    }

    const unsigned char* p = (unsigned char*)data.c_str();
    const unsigned char* e = p + data.size();
    string res = "";
    char buf[16];
    while (p != e)
    {
        if (! IS_XML_FORBID(*p) && isprint(*p))
        {
            res += *p;
        }
        else
        {
            res += "&#x";
            snprintf(buf, sizeof(buf), "%x", *p);
            res += buf;
            res += ";";
        }
        ++p;
    }
    return res;
}

static map<char, char> GenEscChar()
{
    map<char, char> mp;
    mp['a'] = '\a';
    mp['b'] = '\b';
    mp['t'] = '\t';
    mp['n'] = '\n';
    mp['v'] = '\v';
    mp['f'] = '\f';
    mp['r'] = '\r';
    mp['\\'] = '\\';
    mp['\"'] = '\"';
    mp['\''] = '\'';
    return mp;
}

static map<char, char> s_charVal = GenEscChar();

static inline int xtoi(const char *&p, const char* q)
{
    if (*p == '\\')
    {
        ++p;
        map<char,char>::iterator it = s_charVal.find(*p);
        if (it != s_charVal.end())
        {// 普通字符
            ++p;
            return it->second;
        }
        int res = 0;
        if (*p != 'x')
        {// 八进制
            for (int i = 0; i < 3 && p < q; ++i)
            {
                if (*p >= '0' && *p <= '9')
                {
                    res = res * 8 + (*p - '0');
                    ++p;
                    continue;
                }
                break;
            }
        }
        else 
        {//十六进制
            ++p;
            for (int i = 0; i < 2 && p < q; ++i)
            {
                if (*p >= '0' && *p <= '9')
                {
                    res = res * 16 + (*p - '0');
                    ++p;
                    continue;
                }
                char c = *p | 0x20;
                if (c >= 'a' && c <= 'f')
                {
                    res = res * 16 + (c - 'a' + 10);
                    ++p;
                    continue;
                }
                break;
            }
        }
        return res;
    }
    else
    {
        return *p++;
    }
}

string Str::Esc(const string& data)
{
    const char* p = data.c_str();
    const char* pe = p + data.size();
    string res = "";
    res.reserve(data.size());
    while (p < pe)
    {
        res += xtoi(p, pe);
    }
    return res;
}

typedef unsigned char uchar;

static string _B64Encode(const string& data, int type, bool ap)
{
    static uchar   basis64[2][65] = {
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" }};
    uchar* b64 = basis64[type];
    size_t len = data.size();
    uchar* s = (uchar*)data.c_str();
    string dst = "";
    dst.reserve((len+2)/3 * 4);
    while (len > 2) 
    {
        dst.append(1, b64[(s[0] >> 2) & 0x3f]);
        dst.append(1, b64[((s[0] & 3) << 4) | (s[1] >> 4)]);
        dst.append(1, b64[((s[1] & 0x0f) << 2) | (s[2] >> 6)]);
        dst.append(1, b64[s[2] & 0x3f]);

        s += 3;
        len -= 3;
    }
    if (len) 
    {
        dst.append(1, b64[(s[0] >> 2) & 0x3f]);

        if (len == 1) 
        {
            dst.append(1, b64[(s[0] & 3) << 4]);
            if (ap) 
            {
                dst.append(1, '='); 
            }
        } 
        else 
        {
            dst.append(1, b64[((s[0] & 3) << 4) | (s[1] >> 4)]);
            dst.append(1, b64[(s[1] & 0x0f) << 2]);
        }
        if (ap) 
        {
            dst.append(1, '='); 
        }
    }
    return dst;
}


string Str::B64Enc(const string& data)
{
    return _B64Encode(data, 1, true);
}

string Str::B64EncNoAppend(const string& data)
{
    return _B64Encode(data, 1, false);
}

string Str::SafeB64Enc(const string& data)
{
    return _B64Encode(data, 0, true);
}

string Str::SafeB64EncNoAppend(const string& data)
{
    return _B64Encode(data, 0, false);
}

static string _B64Decode(const string& data, int type, bool ap)
{
    static u_char _basis[2][256] = {
        {
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
            77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63, 
            77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77, 

            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
        },
        {
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63, 
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
            77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77, 
            77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77, 

            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
        }
    };
    u_char* basis = _basis[type];
    size_t len;
    u_char *s; 
    string src = data;

    for (len = 0; len < src.size(); len++) 
    {
        if (src[len] == '=') 
        {
            break;
        }

        if (basis[(int)src[len]] == 77) 
        {
            return "";
        }
    }

    if (! ap)
    {
        if (src.size() % 4 == 3) 
        {
            src += "=";
        } 
        else if (src.size() % 4 == 2) 
        {
            src += "==";
        }
    }
    else if (len % 4 == 1) 
    {
        return "";
    }

    s = (u_char*)src.c_str();

    string dst;
    dst.reserve((src.size() + 3)/4 * 3);
    while (len > 3) {
        dst.append(1, (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4));
        dst.append(1, (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2));
        dst.append(1, (u_char) (basis[s[2]] << 6 | basis[s[3]]));

        s += 4;
        len -= 4;
    }
    if (len > 1) {
        dst.append(1, (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4));
    }

    if (len > 2) {
        dst.append(1, (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2));
    }
    return dst;
}

string Str::B64Dec(const string& data)
{
    return _B64Decode(data, 1, true);
}

string Str::B64DecNoAppend(const string& data)
{
    return _B64Decode(data, 1, false);
}

string Str::SafeB64Dec(const string& data)
{
    return _B64Decode(data, 0, true);
}

string Str::SafeB64DecNoAppend(const string& data)
{
    return _B64Decode(data, 0, false);
}

bool Str::AtoI(const string& s, int& r)
{
    if (s.empty())
    {
        return false;
    }
    r = 0;
    sscanf(s.c_str(), "%d", &r);

    char buf[16];
    sprintf(buf, "%d", r);
    if (s == string(buf))
    {
        return true;
    }
    return false;
}
bool Str::AtoF(const string& s, double& r)
{
    if (s.empty())
    {
        return false;
    }
    r = 0;
    sscanf(s.c_str(), "%lf", &r);

    char buf[16];
    sprintf(buf, "%lf", r);
    if (s == string(buf, s.size()))
    {
        return true;
    }
    return false;
}

string Str::ToKMGT(double v)
{
    char buf[32];
    if ((1000 - v) > 0.1) 
    {
        snprintf(buf, sizeof(buf), "%6.2f", v);
    }
    else if ((1000 - v / 1024) > 0.1) 
    {
        snprintf(buf, sizeof(buf), "%5.1f%s", v / 1024, "K");
    } 
    else if ((1000 - v / 1024 / 1024) > 0.1) 
    {
        snprintf(buf, sizeof(buf), "%5.1f%s", v / 1024 / 1024, "M");
    } 
    else if ((1000 - v / 1024 / 1024 / 1024) > 0.1) 
    {
        snprintf(buf, sizeof(buf), "%5.1f%s", v / 1024 / 1024 / 1024, "G");
    } 
    else if ((1000 - v / 1024 / 1024 / 1024 / 1024) > 0.1) 
    {
        snprintf(buf, sizeof(buf), "%5.1f%s", v / 1024 / 1024 / 1024 / 1024, "T");
    }
    return buf;
}

#ifdef _JUST_TEST

#include <iostream>

int main(int argc, char* argv[])
{
    string s = argv[1];
    EList<string> arr;
    Str::SplitEscaped(s, arr);
    string* p;
    for (p = arr.begin(); ! arr.end(); p = arr.next())
    {
        cout << *p << endl;
    }

    int i;
    double d;
    if (Str::AtoI(argv[1], i))
    {
        cout << "int:" << i << endl;
    }
    else if (Str::AtoF(argv[1], d))
    {
        cout << "double:" << d << endl;
    }
    return 0;
/*    char buf[1024];
    int len = fread(buf, 1, 1024, stdin);
    string s(buf, len);

    cout << "Str::B64Enc: " << (s = Str::B64Enc(s)) << endl;
    cout << "Str::B64Dec: " << (s = Str::B64Dec(s)) << endl;
    cout << "Str::B64EncNoAppend: " << (s = Str::B64EncNoAppend(s)) << endl;
    cout << "Str::B64DecNoAppend: " << (s = Str::B64DecNoAppend(s)) << endl;
    cout << "Str::SafeB64Enc: " << (s = Str::SafeB64Enc(s)) << endl;
    cout << "Str::SafeB64Dec: " << (s = Str::SafeB64Dec(s)) << endl;
    cout << "Str::SafeB64EncNoAppend: " << (s = Str::SafeB64EncNoAppend(s)) << endl;
    cout << "Str::SafeB64DecNoAppend: " << (s = Str::SafeB64DecNoAppend(s)) << endl;
*/
}
#endif
