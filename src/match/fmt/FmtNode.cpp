#include <sys/time.h>
#include "FmtNode.h"
#include "Log.h"
#include "Str.h"
#include "DatNode.h"
#include "Zlib.h"
#include "File.h"
#include "System.h"
#include "Var.h"
using namespace std;

FmtNode::FmtNode() : 
    m_varName(NULL),
    m_fixLen(NULL),
    m_repeatedSplit(NULL),
    m_split(NULL),
    m_subDefSplit(NULL),
    m_from(FROM_BODY),
    m_to(0),
    m_type(NVT_NONE)
{
}

FmtNode::~FmtNode()
{
}

bool FmtNode::LoadAttr(TiXmlElement* e)
{
    const char* p = NULL;

    //获取var属性,必须以$开头
    m_varName = e->Attribute("var");
    if (m_varName && *m_varName != '$')
    {
        GLOG(IM_ERROR, "var \"%s\" must begin by '$'", m_varName);
        return false;
    }
    //获取length属性
    m_fixLen = e->Attribute("length");
    if (m_fixLen)
    {//校验合法性，要么为数字，要么为变量名
        int tmp = atoi(m_fixLen);
        if (0 == tmp && *m_fixLen != '0')
        {//不是数字
            if (*m_fixLen != '$')
            {//不是变量
                GLOG(IM_ERROR, "length is not int");
                return false;
            }
        }
        else if (tmp <= 0)
        {
            GLOG(IM_ERROR, "length must > 0");
            return false;
        }
    }
    //获取repeated split属性
    m_repeatedSplit = e->Attribute("repeated_split");

    //获取split属性
    m_split = e->Attribute("split");
    //获取sub def split属性
    m_subDefSplit = e->Attribute("sub_def_split");
    //获取from属性
    p = e->Attribute("from");
    if (p)
    {
        m_from = 0;
        EList<string> v;
        Str::SplitTrimNoNull(p, v, "|");
        for (string* it = v.begin(); ! v.end(); it = v.next())
        {
            if ("HTTP_HEAD" == *it)
            {
                m_from |= FROM_HTTP_HEAD;
            }
            else if ("HTTP_CODE" == *it)
            {
                m_from |= FROM_HTTP_CODE;
            }
            else if ("HTTP_METHOD" == *it)
            {
                m_from |= FROM_HTTP_METHOD;
            }
            else if ("ARPC_METHOD" == *it)
            {
                m_from |= FROM_ARPC_METHOD;
            }
            else if ("ARPC_FAILED_MSG" == *it)
            {
                m_from |= FROM_ARPC_FAILED_MSG;
            }
            else if ("HTTP_URI_PATH" == *it)
            {
                m_from |= FROM_HTTP_URI_PATH;
            }
            else if ("HTTP_URI_QUERY" == *it)
            {
                m_from |= FROM_HTTP_URI_QUERY;
            }
            else if ("NULL" == *it)
            {
                m_from |= FROM_NULL;
            }
            else if ("BODY" == *it)
            {
                m_from |= FROM_BODY;
            }
            else
            {
                GLOG(IM_ERROR, "no support \"%s\"", it->c_str());
                return false;
            }
        }
    }

    //获取type属性
    p = e->Attribute("type");
    if (p)
    {
        EList<string> v;
        Str::SplitTrimNoNull(p, v, "|");
        for (string* it = v.begin(); ! v.end(); it = v.next())
        {
            if ("INT" == *it)
            {
                m_type |= NVT_INT;
            }
            else if ("JARR" == *it)
            {
                m_type |= NVT_JARR;
            }
            else if ("JLEAF" == *it)
            {
                m_type |= NVT_JLEAF;
            }
            else
            {
                GLOG(IM_ERROR, "no support \"%s\" for \"type\" attr", p);
                return false;
            }
        }
    }
    p = e->Attribute("op");
    if (p)
    {
        EList<string> v;
        Str::SplitTrimNoNull(p, v, ",");
        for (string* it = v.begin(); ! v.end(); it = v.next())
        {
            if (! LoadOP(*it, e))
            {
                return false;
            }
        }
    }
    p = e->Attribute("to");
    if (p)
    {
        m_to = 0;
        string s = p;
        if (0 == strcmp(p, "HTTP_METHOD"))
        {
            m_to |= TO_HTTP_METHOD;
        }
        else if (0 == strcmp(p, "HTTP_URI_PATH"))
        {
            m_to |= TO_HTTP_URI_PATH;
        }
        else if (0 == strcmp(p, "HTTP_URI_QUERY"))
        {
            m_to |= TO_HTTP_URI_QUERY;
        }
        else if (0 == strcmp(p, "HTTP_HEAD"))
        {
            m_to |= TO_HTTP_HEAD;
        }
        else if (0 == strcmp(p, "HTTP_VERSION_MINOR"))
        {
            m_to |= TO_HTTP_VERSION_MINOR;
        }
        else if (0 == strcmp(p, "HTTP_CODE"))
        {
            m_to |= TO_HTTP_CODE;
        }
        else if (0 == strcmp(p, "ARPC_FAILED_MSG"))
        {
            m_to |= TO_ARPC_FAILED_MSG;
        }
        else if (0 == strcmp(p, "ARPC_METHOD"))
        {
            m_to |= TO_ARPC_METHOD;
        }
        else if (0 == strcmp(p, "ARPC_SERVICE"))
        {
            m_to |= TO_ARPC_SERVICE;
        }
        else if (0 == strcmp(p, "HTTP_CODE_MSG"))
        {
            m_to |= TO_HTTP_CODE_MSG;
        }
        else
        {
            GLOG(IM_ERROR, "no support \"%s\" for \"to\" attr", p);
            return false;
        }
    }
    return true;
}

bool FmtNode::GetNodeStr(RRMessage* msg, 
        const string& body,
        int& idx, 
        EList<string>& src, bool& useIdx)
{
    useIdx = false;
#define PUSH_NONULL(src, s) \
    do { \
        string tk=s; \
        TokOP(msg, tk); \
        { src.push_back(tk); } \
    }while(0)

    if (m_from & FROM_HTTP_CODE)
    {
        PUSH_NONULL(src, msg->GetHttpCode());
    }
    if (m_from & FROM_NULL)
    {
        PUSH_NONULL(src, "");
    }
    if (m_from & FROM_HTTP_METHOD || m_from & FROM_ARPC_METHOD)
    {
        PUSH_NONULL(src, msg->GetMethod());
    }
    if (m_from & FROM_ARPC_SERVICE)
    {
        PUSH_NONULL(src, msg->GetService());
    }
    if (m_from & FROM_ARPC_FAILED_MSG)
    {
        PUSH_NONULL(src, msg->GetFailedMsg());
    }
    if (m_from & FROM_HTTP_URI_PATH)
    {
        PUSH_NONULL(src, msg->GetUriPath());
    }
    if (m_from & FROM_HTTP_URI_QUERY)
    {
        PUSH_NONULL(src, msg->GetUriQuery());
    }
    if (m_from & FROM_HTTP_HEAD)
    {
        PUSH_NONULL(src, msg->GetHeader());
    }
    if (m_from & FROM_BODY)
    {
        string b = "";
        useIdx = true;
        if (GetTok(msg, body, idx, b))
        {
            GLOG(IM_DEBUG, "get token:%s", Str::ToPrint(b).c_str());
            TokOP(msg, b);
            src.push_back(b);
        }
        else
        {
            GLOG(IM_ERROR, "get token error, body=%s, idx=%d", 
                    body.c_str(), idx);
            return false;
        }
    }
    return true;
}

bool FmtNode::ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add)
{
    if (m_subs.size() == 0)
    {
        return true;
    }

    int idx = 0;
    bool useIdx = false;
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        EList<string> v;
        bool ui = false;
        if (! ((FmtNode*)(*it))->GetNodeStr(msg, dns.s, idx, v, ui))
        {
            return false;
        }
        if (ui)
        {// 如果用过index,则标记为使用过idx
            useIdx = true;
        }
        EList<_DNS> vs;
        if (! ((FmtNode*)(*it))->ParseDG(msg, vs, &v))
        {
            return false;
        }
        for (_DNS* i = vs.begin(); ! vs.end(); i = vs.next())
        {
            //增加子节点
            dns.dn->AddChild(i->dn);
        }
    }
    //if (0 != idx && 0 != m_subs.size() && -1 != idx && idx < (int)dns.s.size())
    if (idx < 0 || (idx == 0 && useIdx == false))// 解析完成
    {
    }
    else 
    {//未解析完全
        GLOG(IM_ERROR, "no parse done, remain:%s",
                dns.s.substr(idx).c_str());
        return false;
    }
    return true;
}

bool FmtNode::ParseDG(RRMessage* msg, 
        EList<_DNS>& vdns, EList<string>* src)
{
    GLOG(IM_DEBUG, "begin parse:%s", m_name.c_str());
    if (! ParseOne(msg, vdns, src))
    {//本字段
        GLOG(IM_ERROR, "parse one failed.");
        return false;
    }
    EList<_DNS> add;
    for (_DNS* idns = vdns.begin(); ! vdns.end(); idns = vdns.next())
    {//重复字段
        //有子字段，则挨个解析
        if (! ParseSubNode(msg, *idns, &add))
        {
            goto FAILED;
        }
    }
    for (_DNS* idns = add.begin(); ! add.end(); idns = add.next())
    {// 对新增加的节点挂载
        vdns.push_back(*idns);
    }
    GLOG(IM_DEBUG, "end parse:%s", m_name.c_str());
    return true;
FAILED:
    for (_DNS* j = vdns.begin(); ! vdns.end(); j = vdns.next())
    {
        delete j->dn;
    }
    for (_DNS* j = add.begin(); ! add.end(); j = add.next())
    {
        delete j->dn;
    }
    vdns.clear();
    add.clear();
    return false;
}

bool FmtNode::Parse(RRMessage* msg)
{
    GLOG(IM_DEBUG, "begin parse---------");
    // 清除变量
    msg->ClearVars();
    EList<string> v;
    int idx = 0;
    bool useIdx = false;
    if (! GetNodeStr(msg, msg->GetBody(), idx, v, useIdx))
    {
        GLOG(IM_DEBUG, "end parse(failed)---------");
        return false;
    }
    EList<_DNS> vs;
    if (! ParseDG(msg, vs, &v))
    {
        GLOG(IM_DEBUG, "end parse(failed)---------");
        return false;
    }
    for (_DNS* it = vs.begin(); ! vs.end(); it = vs.next())
    {
        msg->AddNode(it->dn);
    }
    msg->SetFmtNode(this);
    GLOG(IM_DEBUG, "end parse(success)---------");
    GLOG(IM_DEBUG, "%s\n(done)", msg->DebugString().c_str());
    return true;
}

bool FmtNode::GenNode(RRMessage* msg,
        EList<_DNS>& res,
        const string& src)
{
    _DNS dns;
    dns.dn = new DatNode();
    dns.s = src;
    dns.dn->SetName(m_name);
    if (! FmtNode::FillValue(msg, dns.dn, dns.s))
    {
        delete dns.dn;
        return false;
    }
    res.push_back(dns);
    return true;
}

int FmtNode::GetOneIdx(RRMessage* msg, const string& src, int& pos)
{
    if (pos >= (int)src.length())
    {
        // 结束
        return 0;
    }
    for (Node** i = m_subs.begin(); ! m_subs.end(); i = m_subs.next())
    {
        EList<string> s;
        bool useIdx = false;
        if (((FmtNode*)(*i))->GetNodeStr(msg, src, pos, s, useIdx))
        {
            if (0 != s.size() && ((FmtNode*)(*i))->m_varName)
            {
                msg->AddVar(((FmtNode*)(*i))->m_varName, *s.begin());
            }
        }
        else
        {//出错
            return -1;
        }
    }
    // 继续
    return 1;
}

void FmtNode::ClearAllVars(RRMessage* msg)
{
    if (! msg)
    {
        return ;
    }
    if (m_varName)
    {
        msg->DelVar(m_varName);
    }
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        ((FmtNode*)(*it))->ClearAllVars(msg);
    }
    return ;
}

bool FmtNode::ParseOne(RRMessage* msg, 
        EList<_DNS>& res, EList<string>* src)
{
    // 清除本节点以内的所有变量
    ClearAllVars(msg);
    if (0 == src->size())
    {
        return false;
    }
    if (! m_repeatedSplit)
    {//单独一个,或者分割符没有，但是多值
        return GenNode(msg, res, *(src->begin()));
    }
    for (string* it = src->begin(); ! src->end(); it = src->next())
    {
        if (0 == it->size())
        {
            continue;
        }
        EList<string> v;
        if (*m_repeatedSplit == '\0')
        {// 分隔符没有，但是多值
            int idx = 0;
            int oldIdx= 0;
            while (true)
            {
                oldIdx = idx;
                int ret = GetOneIdx(msg, *it, idx);
                if (ret < 0)
                {
                    return false;
                }
                else if (ret == 0)
                {
                    break;
                }
                else
                {
                    v.push_back(it->substr(oldIdx, idx - oldIdx));
                }
            }
        }
        else
        {
            Str::SplitTrimNoNull(*it, v, m_repeatedSplit);
        }
        for (string* i = v.begin(); ! v.end(); i = v.next())
        {
            if (! GenNode(msg, res, *i))
            {
                for (_DNS* j = res.begin(); ! res.end(); j = res.next())
                {
                    delete j->dn;
                }
                res.clear();
                return false;
            }
        }
    }
    return true;
}

bool FmtNode::GetTok(
        RRMessage* msg, 
        const string& body,
        int& idx, 
        string& tok)
{
    bool res = false;
    const char* split = NULL;
    int fixLen = 0;
    if (m_fixLen)
    {//取固定长度
        if ('$' == *m_fixLen)
        {
            bool found = false;
            fixLen = atoi(msg->GetVar(m_fixLen, found));
        }
        else
        {
            fixLen = atoi(m_fixLen);
        }
    }
    else if (NVT_INT & m_type)
    {
        fixLen = sizeof(int);
    }
    if (fixLen > 0)
    {
        GLOG(IM_DEBUG, "token fix len:%d", fixLen);
        res = Str::Tok(body, idx, tok, fixLen);
        goto END;
    }
    if (m_split)
    {
        split = m_split;
    }
    else if (m_parent)
    {
        FmtNode* p = (FmtNode*)m_parent;
        if (p->m_subDefSplit)
        {
            split = p->m_subDefSplit;
        }
    }
    GLOG(IM_DEBUG, "token split:%s", split);
    res = Str::Tok(body, idx, tok, split);
END:
//    TokOP(msg, res);
    return res;
}

bool FmtNode::TokOP(RRMessage* msg,
        string& tok)
{
    ExecOP(tok, msg, tok);
    return true;
}

bool FmtNode::FillValue(RRMessage* msg, DatNode* dn, const string& src)
{
    //if (m_subs.empty())
    {
        string v = src;
        if (NVT_INT & m_type)
        {
            char buf[16];
            sprintf(buf, "%d", *(int*)src.c_str());
            v = buf;
        }
        /*
        if (m_bop & BOP_URL_ENC)
        {
            v = Str::UrlEnc(v);
        }
        if (m_bop & BOP_URL_DEC)
        {
            v = Str::UrlDec(v);
        }*/
        dn->SetValue(v);
        if (m_varName)
        {
            msg->AddVar(m_varName, v);
        }
        GLOG(IM_DEBUG, "%s=%s", dn->GetName().c_str(), Str::ToPrint(v).c_str());
    }
    return true;
}

bool FmtNode::Serialize(RRMessage* dst, 
        EList<DatNode*>& dn, string& s, RRMessage* src)
{
    //多个dn之间，是repeated的
    string rs = "";
    for (DatNode** it = dn.begin(); ! dn.end(); it = dn.next())
    {
        string v;
        if ((*it)->Hide())
        {
            continue;
        }
        if (! SerializeOne(dst, *it, v, src))
        {
            return false;
        }
        TokOP(src, v);
        s += rs + v;
        if (m_repeatedSplit)
        {
            rs = m_repeatedSplit;
        }
    }
    return true;
}

void FmtNode::DirectFill(RRMessage* msg, DatNode* dn, string& s)
{
    string v = dn->GetValue(msg);
    if (NVT_INT & m_type)
    {
        int i = atoi(v.c_str());
        char buf[16];
        memcpy(buf, &i, sizeof(int));
        s = string(buf, sizeof(int));
    }
    else
    {
        s = v;
    }
}

bool FmtNode::SerializeOne(RRMessage* dst, 
        DatNode* dn, string& s, RRMessage* src)
{
    if (m_to & TO_HTTP_VERSION_MINOR)
    {
        dst->SetHttpVersionMinor(atoi(dn->GetValue(src).c_str()));
        return true;
    }
    else if (m_to & TO_HTTP_METHOD)
    {
        dst->SetMethod(dn->GetValue());
        return true;
    }
    else if (m_to & TO_HTTP_URI_PATH)
    {
        dst->SetUriPath(dn->GetValue());
        return true;
    }
    else if (m_to & TO_HTTP_CODE)
    {
        dst->SetHttpCode(dn->GetValue(src));
        return true;
    }
    else if (m_to & TO_HTTP_CODE_MSG)
    {
        dst->SetHttpCodeMsg(dn->GetValue(src));
        return true;
    }
    else if (m_to & TO_ARPC_FAILED_MSG)
    {
        dst->SetFailedMsg(dn->GetValue(src));
        return true;
    }
    else if (m_to & TO_ARPC_METHOD)
    {
        dst->SetMethod(dn->GetValue(src));
        return true;
    }
    else if (m_to & TO_ARPC_SERVICE)
    {
        dst->SetService(dn->GetValue(src));
        return true;
    }
    else if (m_to & TO_HTTP_URI_QUERY)
    {
        return true;
    }
    else if (m_to & TO_HTTP_HEAD)
    {
        return true;
    }
    else if (dn->NoPack() || 0 == m_subs.size())
    {
        DirectFill(src, dn, s);
        return true;
    }
    return SerializeSub(dst, dn, s, src);
}

int FmtNode::GetType()
{
    return m_type;
}

FmtNode* FmtNode::GetByName(DatNode* dn)
{
    return (FmtNode*)GetSub(dn->GetName());
}

bool FmtNode::GetAllSubVals(RRMessage* dst,
        DatNode* dn, string& s, RRMessage* src)
{
    EList<Node*>& vec = dn->GetSubs();
    int idx = 0;
    for (Node** j = vec.begin(); ! vec.end(); j = vec.next())
    {
        DatNode* d = (DatNode*)*j;
        if (d->Hide())
        {
            continue;
        }
        string v = "";
        FmtNode* f = GetByName(d);
        if (! f)
        {
            v = d->GetValue(src);
        }
        else
        {
            EList<DatNode*> sv;
            sv.push_back(d);
            if (! f->Serialize(dst, sv, v, src))
            {
                return false;
            }
        }
        GetOneSubVal(d, v, s, dst, idx);
        ++idx;
    }
    return true;
}

bool FmtNode::GetOneSubVal(DatNode* d, string& v,
        string& s, RRMessage* msg, int idx)
{
    if (m_to & TO_HTTP_HEAD)
    {
        msg->AddHttpHead(d->GetName(), v);
        GLOG(IM_DEBUG, "add http head:%s:%s",
                d->GetName().c_str(), v.c_str());
        return true;
    }
    else if (m_to & TO_HTTP_URI_QUERY)
    {
        msg->AddHttpUriQuery(d->GetName(), v);
        GLOG(IM_DEBUG, "add uri query:%s:%s",
                d->GetName().c_str(), v.c_str());
        return true;
    }
    return false;
}

bool FmtNode::SerializeSub(RRMessage* dst, 
        DatNode* dn, string& s, RRMessage* src)
{
    if (m_to & TO_HTTP_HEAD || m_to & TO_HTTP_URI_QUERY)
    {
        return GetAllSubVals(dst, dn, s, src);
    }
    string rsp = "";
    string allsubs = "";
    for (Node** i = m_subs.begin(); ! m_subs.end(); i = m_subs.next())
    {
        string subs = "";
        FmtNode* f = (FmtNode*)*i;
        EList<Node*>& v = dn->GetSubs();
        EList<DatNode*> vd;
        bool allHide = true;
        bool found = false;
        for (Node** j = v.begin(); ! v.end(); j = v.next())
        {
            DatNode* d = (DatNode*)*j;
            if (d->GetName() == f->GetName())
            {
                found = true;
                if (! d->Hide())
                {//未隐藏
                    allHide = false;
                    vd.push_back(d);
                }
            }
        }
        if (0 != vd.size())
        {
            if (! f->Serialize(dst, vd, subs, src))
            {
                return false;
            }
        }
        if (found && allHide)
        {//全都hide，那么不能有分割符
            continue;
        }
        allsubs += rsp + subs;
        if (f->m_split)
        {
            rsp = f->m_split;
        }
        else if (m_subDefSplit)
        {
            rsp = m_subDefSplit;
        }
        else
        {
            rsp = "";
        }
    }
    dn->ExecOP(allsubs, src, allsubs);
    s += allsubs;
    return true;
}
