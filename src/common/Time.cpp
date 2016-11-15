#include <cstddef>
#include <stdlib.h>
#include "Time.h"

using namespace std;

Time::Time()
{
    gettimeofday(&m_tv, NULL);
}

bool Time::operator < (const Time& t) const
{
    return LT(m_tv, t.m_tv);
}

int Time::operator - (const Time& t) const
{
    return (m_tv.tv_sec - t.m_tv.tv_sec) * 1000000 + 
        (m_tv.tv_usec - t.m_tv.tv_usec);
}

int Time::StrToUS(const string& s)
{
    if (s.empty())
    {
        return -1;
    }
    string::size_type p = s.find_first_not_of("0123456789");
    if (string::npos == p || 0 == p)
    {//没找到任何单位,或者第一个字符就是单位
        return -1;
    }
    string unit = s.substr(p);
    int scale = 1;
    if ("m" == unit)
    {
        scale = 60 * 1000 * 1000;
    }
    else if ("s" == unit)
    {
        scale = 1000 * 1000;
    }
    else if ("ms" == unit)
    {
        scale = 1000;
    }
    else if ("us" != unit)
    {//不认识的单位
        return -1;
    }
    int num = atoi(s.c_str());
    if (num < 0)
    {//不能是负数
        return -1;
    }
    return num * scale;
}

void Time::AddUS(struct timeval& t, int us)
{
    t.tv_usec += us;
    if (t.tv_usec > 1000000)
    {
        t.tv_sec += t.tv_usec / 1000000;
        t.tv_usec %= 1000000;
    }
}

bool Time::LT(const struct timeval& t1, const struct timeval& t2)
{
    if (t1.tv_sec < t2.tv_sec)
    {
        return true;
    }
    else if (t1.tv_sec > t2.tv_sec)
    {
        return false;
    }
    else
    {
        return (t1.tv_usec < t2.tv_usec);
    }
}
