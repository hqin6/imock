/**
 * 文件名称：Reg.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2013.12.22
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _REG_H_
#define _REG_H_

#include <sys/types.h>
#include <regex.h>
#include <string>
#include "Alloc.h"

class Reg : public Alloc
{
    public:
        //
        Reg(const std::string& s);
        //构造具有s的正则
        bool Init();
        //匹配
        bool Match(const std::string& s) const;
        //获取正则表达式
        const std::string& GetPattern()const;

    private:
        regex_t m_reg;
        std::string m_regs;
};


#endif
