/**
 * 文件名称：PBFmtNode.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.20
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _PBFMTNODE_H_
#define _PBFMTNODE_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
#include "FmtNode.h"

class PBFmtNode : public FmtNode
{
    public:
        PBFmtNode();
        ~PBFmtNode();
        bool LoadAttr(TiXmlElement* e);
        virtual bool FillValue(RRMessage* rrm, 
                DatNode* dn, const std::string& src);
    protected:
        virtual bool ParseSubNode(RRMessage* rrm, _DNS& dns, EList<_DNS>* add = NULL);
        virtual bool SerializeOne(RRMessage* dst, 
                DatNode* dn, std::string& s, RRMessage* src);
        virtual bool SerializeSub(RRMessage* dst, 
                FmtNode* fn, DatNode* dn, 
                google::protobuf::Message* msg, RRMessage* src);
    private:
        bool ParseSubNodePB(RRMessage* rrm, 
                _DNS& dns, 
                const google::protobuf::Message& msg,
                Node* pf);
    private:
        //static bool MergeFmtToPBNode(FmtNode* fn, DatNode* dn, RRMessage* rrm);

    private:
        std::string m_pbMsg;
        std::string m_pbFile;

        google::protobuf::compiler::DiskSourceTree m_dst;
        google::protobuf::compiler::Importer* m_ipt;
        const google::protobuf::Descriptor* m_desc;
        google::protobuf::DynamicMessageFactory* m_fcty;
        const google::protobuf::Message* m_msgTpl;
        google::protobuf::Message* m_msg;
};


#endif
