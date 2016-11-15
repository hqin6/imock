#include <string.h>
#include "Var.h"

using namespace std;

const string Var::KEY("$");

const string& Var::GetResult()
{
    if (0 == m_posCopy)
    {
        return m_src;
    }
    return m_result;
}

Var::Var(const std::string& src) 
{
    m_src = src;
    m_posFind = 0;
    m_posCopy = 0;
    m_result.reserve(m_src.size() * 2);
}

string Var::Next(string& o)
{
    if (-1 == m_posCopy)
    {
        return "";
    }
    if ((int)string::npos == m_posFind)
    {
        return "";
    }
    m_posFind = m_src.find(KEY, m_posFind);
    if ((int)string::npos == m_posFind)
    {
        return "";
    }
    if (m_posFind == (int)m_src.size() - 1)
    {
        return "";
    }
    string varName = "";
    int b = m_posFind + 1;
    bool flag = false;
    if (m_src[b] == '{')
    {
        b++;
        flag = true;
    }
    int e = b;
    while (e < (int)m_src.size())
    {
        int c = m_src[e];
        if (! isalnum(c) && c != '_')
        {
            break;
        }
        e++;
    }
    varName = m_src.substr(b, e - b);
    if (flag)
    {// 最后需要是\}
        if (e > (int)m_src.size() - 1 || m_src[e] != '}')
        {
            return "";
        }
        ++e;
    }
    o = m_src.substr(m_posFind, e - m_posFind);
    return Var::KEY + varName;
}

void Var::Replace(int offset, int len, string* dst)
{
    if (-1 == m_posCopy)
    {
        return ;
    }
    if (! dst)
    {// 忽略本次查找的key
        m_posFind += KEY.size();
        return ;
    }
    // 从上一次copy的偏移拷贝
    if (m_posFind + offset > m_posCopy)
    {
        m_result += m_src.substr(m_posCopy, m_posFind + offset - m_posCopy);
    }
    m_result.append(*dst);
    m_posFind = m_posFind + offset + len;
    m_posCopy = m_posFind;
    return ;
}

void Var::ReplaceEnd()
{
    if (-1 == m_posCopy)
    {
        return ;
    }
    if (0 == m_posCopy)
    {// 没有任何宏，直接复制
        return ;
    }
    m_result += m_src.substr(m_posCopy);
    m_posCopy = -1;
}

bool Var::Match(const char* p, string* dst)
{
    const char* pb = &m_src[0];
    const char* pe = pb + m_src.size();
    if (p < pb || p + dst->length() > pe)
    {
        return false;
    }
    if (0 == memcmp(p, dst->c_str(), dst->length()))
    {
        return true;
    }
    return false;
}
