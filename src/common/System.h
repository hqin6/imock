/**
 * 文件名称：System.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.04.27
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <string>
#define SYS_CMD_ARG_MAX_NUM 128

class System
{
    public:
        System();
        const std::string Cmd();
        inline void Add(const char* p)
        {
            if (m_argc < (int)sizeof(m_argv))
            {
                m_argv[m_argc++] = (char*)p;
            }
        }
    private:
        char* m_argv[SYS_CMD_ARG_MAX_NUM];
        int m_argc;

};


#endif
