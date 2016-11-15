#include "KVFmtNode.h"
#include "DatNode.h"
#include "Log.h"
#include "Str.h"

using namespace std;

bool KVFmtNode::LoadAttr(TiXmlElement* e)
{
    if (! FmtNode::LoadAttr(e))
    {
        return false;
    }
    const char* p = NULL;
    p = e->Attribute("fmt");
    if (! p || strlen(p) < 3)
    {
        GLOG(IM_ERROR, "kv fmt must be k(split)v");
        return false;
    }
    m_kvSplit = string(p + 1, strlen(p) - 2);

    p = e->Attribute("kv_repeated_split");
    if (p)
    {
        m_kvRepeatedSplit = p;
    }
    return true;
}

bool KVFmtNode::ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add)
{
    if (dns.s.empty())
    {
        return true;
    }
    EList<string> kv;
    if (! m_kvRepeatedSplit.empty())
    {
        Str::SplitTrimNoNull(dns.s, kv, m_kvRepeatedSplit);
    }
    else
    {
        kv.push_back(dns.s);
    }
    for (string* it = kv.begin(); ! kv.end(); it = kv.next())
    {
        dns.s = *it;
        if (! ParseSubNodeKV(msg, dns))
        {
            return false;
        }
    }
    return true;
}

bool KVFmtNode::ParseSubNodeKV(RRMessage* msg, _DNS& dns)
{
    string k,v;
    int bi = 0;
    if (! Str::Tok(dns.s, bi, k, m_kvSplit.c_str()))
    {
        return false;
    }
    if (! Str::Tok(dns.s, bi, v))
    {
        return false;
    }
    FmtNode* it = (FmtNode*)GetSub(k);
    if (it)
    {
        EList<string> tmp;
        int idx = 0;
        bool useIdx = false;
        if (! it->GetNodeStr(msg, v, idx, tmp, useIdx))
        {
            return false;
        }
        EList<_DNS> vs;
        if (! it->ParseDG(msg, vs, &tmp))
        {
            return false;
        }
        for (_DNS* i = vs.begin(); ! vs.end(); i = vs.next())
        {
            //增加子节点
            dns.dn->AddChild(i->dn);
        }
        GLOG(IM_DEBUG, "add key<%s> = val<%s>", k.c_str(), v.c_str());
        return true;
    }
    //fmt中没有配置，直接填充即可
    DatNode* dn = new DatNode();
    dn->SetName(k);
    dn->SetValue(v);
    dns.dn->AddChild(dn);
    return true;
}

bool KVFmtNode::SerializeSub(RRMessage* dst,
        DatNode* dn, string& s, RRMessage* src)
{
    return GetAllSubVals(dst, dn, s, src);
}

bool KVFmtNode::GetOneSubVal(DatNode* d, string& v,
        string& s, RRMessage* msg, int idx)
{
    if (! FmtNode::GetOneSubVal(d, v, s, msg, idx))
    {
        string kv;
        kv += d->GetName();
        kv += m_kvSplit;
        kv += v;
        if (0 == idx)
        {
            s += kv;
        }
        else
        {
            s += m_kvRepeatedSplit + kv;
        }
    }
    return true;
}

bool KVFmtNode::SerializeOne(RRMessage* dst, 
        DatNode* dn, string& s, RRMessage* src)
{
    if (dn->NoPack())
    {
        DirectFill(src, dn, s);
        return true;
    }
    if (! SerializeSub(dst, dn, s, src))
    {
        return false;
    }
    dn->ExecOP(s, src, s);
    return true;
}

