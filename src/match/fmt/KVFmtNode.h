/**
 * 文件名称：KVFmtNode.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.18
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _KVFMTNODE_H_
#define _KVFMTNODE_H_

#include "FmtNode.h"

class KVFmtNode : public FmtNode
{
    public:
        bool LoadAttr(TiXmlElement* e);
    protected:
        virtual bool GetOneSubVal(DatNode* d, std::string& v,
                std::string& s, RRMessage* msg, int idx);
        virtual bool ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add);
        virtual bool ParseSubNodeKV(RRMessage* msg, _DNS& dns);
        virtual bool SerializeOne(RRMessage* dst,
                DatNode* dn, std::string& s,
                RRMessage* src);
        virtual bool SerializeSub(RRMessage* dst,
                DatNode* dn, std::string& s,
                RRMessage* src);
    private:
        std::string m_kvSplit;
        std::string m_kvRepeatedSplit;
};


#endif
