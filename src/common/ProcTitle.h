/**
 * 文件名称：ProcTitle.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.01.31
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _PROCTITLE_H_
#define _PROCTITLE_H_

#include <string>

//将破坏args： argv
class ProcTitle
{
    public:
        static int SaveEnv(char* const* argv);
        static void Set(const std::string& pname);
        static char* GetMainArgv();
    private:
        static char** s_osArgv;
        static char* s_osArgvLast;
        static char* s_mainArgv;
        
};


#endif
