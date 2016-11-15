/**
 * 文件名称：DiffInfo.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.10.20
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DIFFINFO_H_
#define _DIFFINFO_H_

#include <string>
#include <map>
#include "RRMessage.h"
#include "DatNode.h"
#include "Fmt.h"
#include "DatNodeRoot.h"

class DiffInfo
{
    public:
        DiffInfo(const std::string& name);
        ~DiffInfo();
        bool Init();
        bool ParseFile(DatNode* attrRoot, bool isq = true);
        bool DoDiff(DiffInfo* di, bool brk, std::string& resultStr);
        const char* GetName();

    private:
        DatNode* MergeValueAttr(DatNode* s1, DatNode* s2);
        int Equal(DiffInfo* di, bool brk, std::string& resultStr);
    private:
        std::string m_name;
        std::string m_file;
        std::string m_fmtFile;
        RRMessage   m_msg;
        Fmt         m_fmt;
        std::map<std::string, DatNodeRoot*> m_keyNodes;
};


#endif
