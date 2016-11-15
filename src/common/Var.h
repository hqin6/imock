/**
 * 文件名称：Var.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.08.22
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _VAR_H_
#define _VAR_H_

#include <string>

class Var
{
    public:
        // 构造，传递原始串
        Var(const std::string& src);
        // 获取下一个%%，失败返回NULL
        std::string Next(std::string& o);
        // 替换, 如果dst为空，则表示不予替换，忽略本次查找的KEY
        // 如果要替换为空，则dst->len = 0
        void Replace(int offset, int len, std::string* dst);
        // 在Next返回为NULL的时候调用
        // 拷贝剩余的字符
        void ReplaceEnd();
        // 获取处理后的结果
        const std::string& GetResult();
        // 是否匹配，需要判断边界
        bool Match(const char* p, std::string* dst);
    private:
        // 替换后的最终结果
        std::string m_result;
        // 存储扫描位置
        int m_posFind;
        // 存储拷贝位置
        int m_posCopy;
        // 原始串
        std::string m_src;
    private:
        static const std::string KEY;
};


#endif
