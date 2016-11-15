/**
 * 文件名称：Fmt.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.01
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _FMT_H_
#define _FMT_H_

#include <map>
#include <string>
#include "FmtNode.h"
#include "Alloc.h"

class Fmt : public Alloc
{
    public:
        Fmt();
        ~Fmt();
        bool Load(const char* fmtFile);
        std::map<std::string, Node*>* GetQ();
        std::map<std::string, Node*>* GetA();
    private:
        //请求query的格式, 多种不同的请求格式，可以使用fid区分
        std::map<std::string, Node*> m_mp_fmtQ;
        //应答answer的格式, 多种不同的应答格式，可以使用fid区分
        std::map<std::string, Node*> m_mp_fmtA;
        //减少拷贝
        TiXmlDocument* m_doc;
};

#endif
