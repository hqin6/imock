/**
 * 文件名称：JsonFmtNode.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.07.17
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _JSONFMTNODE_H_
#define _JSONFMTNODE_H_

#include "FmtNode.h"
#include "json/json.h"

class JsonFmtNode : public FmtNode
{
    class JsonValueType{
        friend class JsonFmtNode;
        IMOCK::Json::Value* v;
        int type;
        JsonValueType(IMOCK::Json::Value* _v, int _t);
    };
    public:
        // 由于json本身节点可能是repeated的,所以要重写该方法
        bool Serialize(RRMessage* dst, 
                EList<DatNode*>& dn, 
                std::string& s, RRMessage* src);
    protected:
        bool ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add);
        bool ParseSubNodeJson(RRMessage* msg, _DNS& dns, IMOCK::Json::Value& v,
                Node* node, const std::string& name = "");
        bool SerializeOne(RRMessage* dst, DatNode* dn, 
                std::string& s, RRMessage* src);
        bool SerializeSub(RRMessage* dst, DatNode* dn, 
                RRMessage* src, IMOCK::Json::Value& root, FmtNode* fn);
    private:
        static void FmtJsonNode(IMOCK::Json::Value& root, std::string& v, int type, long op);
        static void FillJsonNode(IMOCK::Json::Value& root, IMOCK::Json::Value& v, bool repeated);
};


#endif
