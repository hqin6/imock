/**
 * 文件名称：DatNodeRoot.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DATNODEROOT_H_
#define _DATNODEROOT_H_

#include <string>
#include "DatNode.h"
#include "RRMessage.h"
#include "Fmt.h"

class FmtNode;

class DatNodeRoot : public DatNode
{
    public:
        DatNodeRoot();
        bool LoadAttr(TiXmlElement* e, std::map<std::string, Node*>* mp, const std::string& fid);
        MATCH_RESULT Match(RRMessage* msg);
        int GetRate();
        bool Serialize(RRMessage* dst, RRMessage* src = NULL, const std::string& fid = "");
        const char* GetDebugId();
        const std::string& GetFid();
    public:
        void SetAttr(Node* n);
    protected:
    public:
        using DatNode::Match;
    protected:
        FmtNode* m_fmtNode;
        std::string m_fid;
        int m_rate;
        int m_usleep;
        const char* m_debugId;
};

#endif
