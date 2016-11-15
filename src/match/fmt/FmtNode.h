/**
 * 文件名称：FmtNode.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.01
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _FMTNODE_H_
#define _FMTNODE_H_

#include "Node.h"
#include "RRMessage.h"

#define FROM_HTTP_METHOD        0x0001
#define FROM_HTTP_URI_PATH      0x0002
#define FROM_HTTP_URI_QUERY     0x0004
#define FROM_HTTP_HEAD          0x0008
#define FROM_BODY               0x0010
#define FROM_HTTP_CODE          0x0020
#define FROM_ARPC_METHOD        0x0040
#define FROM_ARPC_FAILED_MSG    0x0080
#define FROM_ARPC_SERVICE       0x0100
#define FROM_NULL               0x0200

#define TO_HTTP_HEAD            0x0001
#define TO_HTTP_CODE            0x0002
#define TO_HTTP_CODE_MSG        0x0004
#define TO_HTTP_VERSION_MINOR   0x0008
#define TO_HTTP_METHOD          0x0010
#define TO_HTTP_URI_PATH        0x0020
#define TO_HTTP_URI_QUERY       0x0040
#define TO_ARPC_FAILED_MSG      0x0080
#define TO_ARPC_METHOD          0x0100
#define TO_ARPC_SERVICE         0x0200


#define NVT_NONE   0x0001
#define NVT_INT    0x0002
#define NVT_JLEAF  0x0004
#define NVT_JARR   0x0008

struct _DNS
{
    //每个节点
    DatNode* dn;
    //每个节点的数值
    std::string s;
};

class FmtNode : public Node
{
    public:
        FmtNode();
        ~FmtNode();
        virtual bool LoadAttr(TiXmlElement* e);
        //解析msg，原数据存放在msg中
        bool Parse(RRMessage* msg);
        virtual bool Serialize(RRMessage* dst, 
                EList<DatNode*>& dn, 
                std::string& s, RRMessage* src);
    protected:
        virtual bool SerializeSub(RRMessage* dst,
                DatNode* dn, std::string& s,
                RRMessage* src);
        virtual bool SerializeOne(RRMessage* dst,
                DatNode* dn, std::string& s,
                RRMessage* src);
        void DirectFill(RRMessage* msg, DatNode* dn, std::string& s);
    public:
        //递归parse，返回根节点
        //req中包含基本信息
        //src是本节点所对应的src串
        virtual bool ParseDG(RRMessage* msg, 
                EList<_DNS>& vdns,
                EList<std::string>* src);
    public:
        virtual bool ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add = NULL);
    protected:
        //从src中解析单个节点值
        virtual bool ParseOne(RRMessage* msg, 
                EList<_DNS>& res,
                EList<std::string>* src = NULL);
        //将单个值填充到dn中,基类可继承该方法实现自己的填充方式
        //比如fmt='k:v'这种形式
        virtual bool GenNode(RRMessage* msg,
                EList<_DNS>& res,
                const std::string& src);
    protected:
        virtual bool GetAllSubVals(RRMessage* dst,
                DatNode* dn, std::string& s, RRMessage* src);
        virtual bool GetOneSubVal(DatNode* d, std::string& v,
                std::string& s, RRMessage* msg, int idx);
    public:
        virtual bool FillValue(RRMessage* msg, 
                DatNode* dn, const std::string& src);
    public:
        //从req中获取本字段的值,将值存放在src中
        virtual bool GetNodeStr(RRMessage* msg, 
                const std::string& body,
                int& idx,
                EList<std::string>& src, bool& useIdx);
        int GetType();
    protected:
        //virtual bool GetAllSubValue(RRMessage* msg, 
                //DatNode* d, std::string& v, Response* res);
        FmtNode* GetByName(DatNode* dn);
    private:
        //返回原始字符串
        bool GetTok(RRMessage* msg, 
                const std::string& body,
                int& idx,
                std::string& tok);
        int GetOneIdx(RRMessage* msg, const std::string& src, int& idx);
        void ClearAllVars(RRMessage* msg);
    public:
        bool TokOP(RRMessage* msg, std::string& body);

    protected:
        const char* m_varName; //只在q中支持
        const char* m_fixLen; //只在q中支持
        const char* m_repeatedSplit;
        const char* m_split;
        const char* m_subDefSplit;
        const char* m_shellCmd;
        int m_from; //只在q中支持
        int m_to;
        int m_type;
};


#endif
