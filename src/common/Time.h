/**
 * 文件名称：Time.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.07
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _TIME_H_
#define _TIME_H_

#include <sys/time.h>
#include <string>

class Time
{
    public:
        //构造时，获取当前时间
        Time();
        //提供对比函数
        bool operator < (const Time& t) const;
        int operator - (const Time& t) const;
        static void AddUS(struct timeval& t, int us);
        static int StrToUS(const std::string& s);
        static bool LT(const struct timeval& t1,
                const struct timeval& t2);
        static std::string StrTimeNow();
        static double SubTimeMs(const struct timeval& t1,
                const struct timeval& t2);
    private:

    private:
        struct timeval m_tv;
};


#endif
