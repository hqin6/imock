#include <libgen.h>
#include <stdio.h>
#include "PBFmtNode.h"
#include "DatNode.h"
#include "Log.h"
#include "define.h"
#include "Str.h"
#include "ProtoBuf.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

PBFmtNode::PBFmtNode() :
    m_ipt(NULL),
    m_desc(NULL),
    m_fcty(NULL),
    m_msgTpl(NULL),
    m_msg(NULL)
{
}

PBFmtNode::~PBFmtNode()
{
    /*
     * 动态的Message是我们用DynamicMessageFactory构造出来的，因此销毁Message必须用同一个DynamicMessageFactory。 
     * 动态更新.proto文件时，我们销毁老的并使用新的DynamicMessageFactory，在销毁DynamicMessageFactory之前，必须先删除所有经过它构造的Message。
     * 原理：DynamicMessageFactory里面包含DynamicMessage的共享信息，析构DynamicMessage时需要用到。生存期必须保持Descriptor>DynamicMessageFactory>DynamicMessage
     * 释放顺序必须是：释放所有DynamicMessage，释放DynamicMessageFactory，释放Importer。
     */
    if (m_msg)
    {
        delete m_msg;
    }
    if (m_fcty)
    {
        delete m_fcty;
    }
    if (m_ipt)
    {
        delete m_ipt;
        m_ipt = NULL;
    }
}

bool PBFmtNode::LoadAttr(TiXmlElement* e)
{
    if (! FmtNode::LoadAttr(e))
    {
        return false;
    }
    const char* p = NULL;
    p = e->Attribute("message");
    if (! p)
    {
        GLOG(IM_ERROR, "no attribute \"message\"");
        return false;
    }
    m_pbMsg = p;

    p = e->Attribute("file");
    if (! p)
    {
        GLOG(IM_ERROR, "no attribute \"file\"");
        return false;
    }
    m_pbFile = p;

    string pbInc = "";
    p = e->Attribute("inc");
    if (p)
    {
        pbInc = p;
    }
    /*
    if (0 != access(d.c_str(), R_OK))
    {
        if (d.empty() || d[0] == '/')
        {
            GLOG(IM_ERROR, "read file failed: %s", d.c_str());
            return false;
        }
        d = string(SETUP_ROOT_DIR""PROTO_DIR"/") + d;
        if (0 != access(d.c_str(), R_OK))
        {
            GLOG(IM_ERROR, "read file failed: %s", d.c_str());
            return false;
        }
    }*/
    const FileDescriptor* fd = NULL;
    if (! ProtoBuf::ImportFile(m_pbFile, pbInc, m_dst, m_ipt, fd))
    {
        GLOG(IM_ERROR, "can't import %s", m_pbFile.c_str());
        return false;
    }

    m_msg = ProtoBuf::CreateMessage(m_pbMsg, m_ipt);
    if (! m_msg)
    {
        GLOG(IM_ERROR, "can't find %s in %s", m_pbMsg.c_str(), m_pbFile.c_str());
        return false;
    }
    return true;
}

bool PBFmtNode::ParseSubNode(RRMessage* rrm, _DNS& dns, EList<_DNS>* add)
{
    Message* msg = m_msg;
    msg->Clear();
    if (! ProtoBuf::ParseFromString(msg, dns.s))
    {
        return false;
    }
    GLOG(IM_INFO, "parse pb.DebugString:%s", msg->DebugString().c_str());
    bool b = ParseSubNodePB(rrm, dns, *msg, this);
    return b;
}

bool PBFmtNode::ParseSubNodePB(RRMessage* rrm, _DNS& dns, const Message& m, Node* pf)
{
#define PARSE_TO_DATNODE(rrm, r, m, f, T, v, op1, op2) \
    do { \
        if (f->label() == FieldDescriptor::LABEL_REPEATED) \
        { \
            int n = r->FieldSize(m, f); \
            for (int i = 0; i < n; ++i) \
            { \
                op1(rrm, r, m, f, T, v, i); \
            }\
        }\
        else \
        { \
            if (r->HasField(m, f)) \
            { \
                op2(rrm, r, m, f, T, v); \
            } \
        } \
    }while (0)

#define _OP_NUM(rrm, r, m, f, T, v, i, g, fmt) \
    { \
        char buf[32]; \
        snprintf(buf, sizeof(buf), fmt, g); \
        v.push_back(buf); \
    }

#define OP_REP_INT32(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%d")

#define OP_INT32(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%d")

#define OP_REP_INT64(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%ld")

#define OP_INT64(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%ld")

#define OP_REP_UINT32(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%u")

#define OP_UINT32(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%u")

#define OP_REP_UINT64(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%lu")

#define OP_UINT64(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%lu")

#define OP_REP_DOUBLE(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%lf")

#define OP_DOUBLE(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%lf")

#define OP_REP_FLOAT(rrm, r, m, f, T, v, i) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->GetRepeated##T(m, f, i), "%f")

#define OP_FLOAT(rrm, r, m, f, T, v) \
    _OP_NUM(rrm, r, m, f, T, v, i, r->Get##T(m, f), "%f")

#define OP_REP_BOOL(rrm, r, m, f, T, v, i) \
    { \
        v.push_back(r->GetRepeated##T(m, f, i) ? "true" : "false"); \
    }

#define OP_BOOL(rrm, r, m, f, T, v) \
    { \
        v.push_back(r->Get##T(m, f) ? "true" : "false"); \
    }

#define OP_REP_ENUM(rrm, r, m, f, T, v, i) \
    { \
        v.push_back(r->GetRepeated##T(m, f, i)->name()); \
    }

#define OP_ENUM(rrm, r, m, f, T, v) \
    { \
        v.push_back(r->Get##T(m, f)->name()); \
    }

#define OP_REP_STR(rrm, r, m, f, T, v, i) \
    { \
        v.push_back(r->GetRepeated##T(m, f, i)); \
    }
#define OP_STR(rrm, r, m, f, T, v) \
    { \
        v.push_back(r->Get##T(m, f)); \
    }

#define OP_REP_MSG(rrm, r, m, f, T, v, i) \
    { \
        DatNode* dn = new DatNode(); \
        string nm = f->name(); \
        dn->SetName(nm); \
        _DNS tmp; \
        tmp.dn = dn; \
        ParseSubNodePB(rrm, tmp, r->GetRepeatedMessage(m, f, i), pf ? pf->GetSub(nm) : NULL); \
        dns.dn->AddChild(dn); \
    }

#define OP_MSG(rrm, r, m, f, T, v) \
    { \
        DatNode* dn = new DatNode(); \
        string nm = f->name(); \
        dn->SetName(nm); \
        _DNS tmp; \
        tmp.dn = dn; \
        ParseSubNodePB(rrm, tmp, r->GetMessage(m, f), pf ? pf->GetSub(nm) : NULL); \
        dns.dn->AddChild(dn); \
    }

    const Reflection *r = m.GetReflection();
    const Descriptor* d = m.GetDescriptor();
    for (int i = 0; i < d->field_count(); i++)
    {
        const FieldDescriptor* f = d->field(i);
        vector<string> v;
        bool objType = false;
        switch (f->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
                PARSE_TO_DATNODE(rrm, r, m, f, Int32, v, OP_REP_INT32, OP_INT32);
                break;
            case FieldDescriptor::CPPTYPE_INT64:
                PARSE_TO_DATNODE(rrm, r, m, f, Int64, v, OP_REP_INT64, OP_INT64);
                break;
            case FieldDescriptor::CPPTYPE_UINT32:
                PARSE_TO_DATNODE(rrm, r, m, f, UInt32, v, OP_REP_UINT32, OP_UINT32);
                break;
            case FieldDescriptor::CPPTYPE_UINT64:
                PARSE_TO_DATNODE(rrm, r, m, f, UInt64, v, OP_REP_UINT64, OP_UINT64);
                break;
            case FieldDescriptor::CPPTYPE_DOUBLE:
                PARSE_TO_DATNODE(rrm, r, m, f, Double, v, OP_REP_DOUBLE, OP_DOUBLE);
                break;
            case FieldDescriptor::CPPTYPE_FLOAT:
                PARSE_TO_DATNODE(rrm, r, m, f, Float, v, OP_REP_FLOAT, OP_FLOAT);
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                PARSE_TO_DATNODE(rrm, r, m, f, Bool, v, OP_REP_BOOL, OP_BOOL);
                break;
            case FieldDescriptor::CPPTYPE_ENUM:
                PARSE_TO_DATNODE(rrm, r, m, f, Enum, v, OP_REP_ENUM, OP_ENUM);
                break;
            case FieldDescriptor::CPPTYPE_STRING:
                PARSE_TO_DATNODE(rrm, r, m, f, String, v, OP_REP_STR, OP_STR);
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                PARSE_TO_DATNODE(rrm, r, m, f, Message, v, OP_REP_MSG, OP_MSG);
                objType = true;
                break;
            default:
                break;
        }

        if (! objType)
        {// 如果不是message,那么叶子节点可以继续解
            string nodeName = f->name();
            FmtNode* fn = NULL;
            if (pf) 
            {
                fn = (FmtNode*)pf->GetSub(nodeName);
            }
            for (vector<string>::iterator vi = v.begin(); vi != v.end(); ++vi)
            {
                if (fn) // 有配置格式节点
                {
                    EList<string> vec;
                    int idx = 0;
                    bool useIdx = false;
                    if (! ((FmtNode*)fn)->GetNodeStr(rrm, *vi, idx, vec, useIdx))
                    {
                        return false;
                    }
                    EList<_DNS> vs;
                    if (!((FmtNode*)fn)->ParseDG(rrm, vs, &vec))
                    {
                        return false;
                    }
                    for (_DNS* ii = vs.begin(); ! vs.end(); ii = vs.next())
                    {
                        //增加子节点
                        dns.dn->AddChild(ii->dn);
                    }
                }
                else // 直接填充
                {
                    DatNode* dn = new DatNode();
                    dn->SetName(nodeName);
                    dn->SetValue(*vi);
                    dns.dn->AddChild(dn);
                }
            }
        }
    }
    return true;
}
/*
bool PBFmtNode::MergeFmtToPBNode(FmtNode* fn, DatNode* dn, RRMessage* rrm)
{
    EList<Node*>& fv = fn->GetSubs();
    EList<Node*>& dv = dn->GetSubs();
    if (0 == dv.size())
    {//dn在pb中是叶子节点
        if (0 == fv.size())
        {//fn在pb中也是叶子节点
            return fn->FillValue(rrm, dn, dn->GetValue(rrm));
        }
        else 
        {//需要进一步解析pb的叶子节点
            _DNS dns;
            dns.s = dn->GetValue(rrm);
            dns.dn = dn;
            return fn->ParseSubNode(rrm, dns);
        }
    }
    for (Node** j = dv.begin(); ! dv.end(); j = dv.next())
    {
        for (Node** i = fv.begin(); ! fv.end(); i = fv.next())
        {
            if ((*i)->GetName() == (*j)->GetName())
            {
                if (! MergeFmtToPBNode((FmtNode*)*i, (DatNode*)*j, rrm))
                {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}
*/

bool PBFmtNode::SerializeOne(RRMessage* dst, 
        DatNode* dn, string& s, RRMessage* src)
{
    if (dn->NoPack())
    {
        DirectFill(src, dn, s);
        return true;
    }
    ::google::protobuf::Message* msg = m_msg;
    msg->Clear();
    bool b = SerializeSub(dst, this, dn, msg, src);
    if (! b)
    {
        return false;
    }
    GLOG(IM_DEBUG, "return pb msg:%s", msg->DebugString().c_str());
    b = msg->SerializeToString(&s);
    dn->ExecOP(s, src, s);
    return b;
}

struct DatElement
{
    EList<DatNode*> d;
    const FieldDescriptor* f;
};

bool PBFmtNode::SerializeSub(RRMessage* dst, 
        FmtNode* fn, DatNode* dn, 
        Message* m, RRMessage* src)
{
#define SERIALIZE_FROM_DATNODE(r, m, f, T, v) \
    do { \
        if (f->label() == FieldDescriptor::LABEL_REPEATED) \
        { \
            r->Add##T(m, f, v);\
        }\
        else \
        { \
            r->Set##T(m, f, v); \
        } \
    }while (0)
    const Reflection* r = m->GetReflection();
    const Descriptor* d = m->GetDescriptor();
    const EnumValueDescriptor* ev = NULL;
    EList<Node*>& vec = dn->GetSubs();
    map<Node*, DatElement*> fmtDatsNode;
    EList<Node*>* fns = NULL;
    if (fn)
    {
        fns = &fn->GetSubs();
    }
    for (Node** i = vec.begin(); ! vec.end(); i = vec.next())
    {//对每一个字段，设置message相应字段的值
        const string v = ((DatNode*)(*i))->GetValue(src);
        const FieldDescriptor* f = d->FindFieldByName((*i)->GetName());
        char tmp[16] = "\0";
        void* buf = (void*)tmp;//避免warning
        bool hasFmt = false;
        FmtNode* subFn = NULL;
        if (! f)
        {
            GLOG(IM_ERROR, "can't recognize \"%s\" for %s", v.c_str(), (*i)->GetName().c_str());
            continue;
        }
        switch (f->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
                sscanf(v.c_str(), "%d", (int32*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, Int32, *(int32*)buf);
                break;
            case FieldDescriptor::CPPTYPE_INT64:
                sscanf(v.c_str(), "%lld", (long long int*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, Int64, *(long long int*)buf);
                break;
            case FieldDescriptor::CPPTYPE_UINT32:
                sscanf(v.c_str(), "%u", (uint32*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, UInt32, *(uint32*)buf);
                break;
            case FieldDescriptor::CPPTYPE_UINT64:
                sscanf(v.c_str(), "%llu", (long long unsigned int*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, UInt64, *(uint64*)buf);
                break;
            case FieldDescriptor::CPPTYPE_DOUBLE:
                sscanf(v.c_str(), "%lf", (double*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, Double, *(double*)buf);
                break;
            case FieldDescriptor::CPPTYPE_FLOAT:
                sscanf(v.c_str(), "%f", (float*)buf);
                SERIALIZE_FROM_DATNODE(r, m, f, Float, *(float*)buf);
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                SERIALIZE_FROM_DATNODE(r, m, f, Bool, 
                        "true" == v ? true : false);
                break;
            case FieldDescriptor::CPPTYPE_ENUM:
                ev = f->enum_type()->FindValueByName(v);
                if (NULL == ev)
                {
                    GLOG(IM_ERROR, "can't recognized %s for enum, ignore", v.c_str());
                }
                else
                {
                    SERIALIZE_FROM_DATNODE(r, m, f, Enum, ev);
                }
                break;
            case FieldDescriptor::CPPTYPE_STRING:
                if (fns)
                {
                    for (Node** j = fns->begin(); ! fns->end(); j = fns->next())
                    {
                        if ((*i)->GetName() == (*j)->GetName())
                        {
                            DatElement* de = NULL;
                            map<Node*, DatElement*>::iterator fmtIt = fmtDatsNode.find(*j);
                            if (fmtIt == fmtDatsNode.end())
                            {
                                de = new DatElement;
                                fmtDatsNode.insert(pair<Node*, DatElement*>(*j, de));
                            }
                            else
                            {
                                de = fmtIt->second;
                            }
                            de->d.push_back((DatNode*)*i);
                            de->f = f;
                            hasFmt = true;
                            break;
                        }
                    }
                }
                if (! hasFmt)
                {//没有配置fmt，则不需要再深层次解析
                    SERIALIZE_FROM_DATNODE(r, m, f, String, v);
                }
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                if (fns)
                {
                    for (Node** j = fns->begin(); ! fns->end(); j = fns->next())
                    {
                        if ((*i)->GetName() == (*j)->GetName())
                        {
                            subFn = (FmtNode*)*j;
                            break;
                        }
                    }
                }
                if (f->label() == FieldDescriptor::LABEL_REPEATED)
                {
                    SerializeSub(dst, subFn, 
                            (DatNode*)*i, r->AddMessage(m, f), src);
                }
                else
                {
                    SerializeSub(dst, subFn, 
                            (DatNode*)*i, r->MutableMessage(m, f), src);
                }
                break;
            default:
                break;
        }
    }
    bool ret = true;
    for (map<Node*, DatElement*>::iterator it = fmtDatsNode.begin();
            it != fmtDatsNode.end(); ++it)
    {
        FmtNode* fn = (FmtNode*)it->first;
        string s;
        if (false == fn->Serialize(dst, it->second->d, s, src))
        {
            ret = false;
            goto END;
        }
        SERIALIZE_FROM_DATNODE(r, m, it->second->f, String, s);
    }
END:
    for (map<Node*, DatElement*>::iterator it = fmtDatsNode.begin();
            it != fmtDatsNode.end(); ++it)
    {
        delete it->second;
    }
    return ret;
}

bool PBFmtNode::FillValue(RRMessage* rrm, DatNode* dn, const string& src)
{
    //字段是一个pb，那么需要继续调用
    _DNS dns;
    dns.s = src;
    dns.dn = dn;
    bool b = ParseSubNode(rrm, dns);
    GLOG(IM_DEBUG, "fill %s done", m_name.c_str());
    return b;
}
