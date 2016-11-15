#include "DatNodeRoot.h"
#include "FmtNode.h"
#include "Log.h"
#include "Time.h"
#include "EList.h"
#include "Str.h"
using namespace std;

DatNodeRoot::DatNodeRoot():
    m_fmtNode(NULL),
    m_rate(-1),
    m_usleep(0),
    m_debugId("")
{
}

const string& DatNodeRoot::GetFid()
{
    return m_fid;
}

bool DatNodeRoot::LoadAttr(TiXmlElement* e, map<string, Node*>* mp, const std::string& fid)
{
    string fmtID = fid;
    if (fmtID.empty())
    {
        const char* id = e->Attribute("fid");
        if (id)
        {
            fmtID = id;
        }
    }
    if (mp && ! m_fmtNode)
    {
        map<string, Node*>::iterator it = mp->find(fmtID);
        if (mp->end() == it)
        {
            GLOG(IM_ERROR, "no fid=%s in fmt", fmtID.c_str());
            return false;
        }
        m_fmtNode = (FmtNode*)it->second;
    }
    m_fid = fmtID;

    const char* p = NULL;
    p = e->Attribute("rate");
    if (p)
    {
        m_rate = atoi(p);
        if (m_rate < 0)
        {
            GLOG(IM_ERROR, "rate = %s < 0", p);
            return false;
        }
    }
    p = e->Attribute("id");
    if (p)
    {
        m_debugId = p;
    }
    p = e->Attribute("sleep");
    if (p)
    {
        int us = Time::StrToUS(p);
        if (us < 0)
        {
            GLOG(IM_ERROR, "\"sleep\" value<%s> format error.", p);
            return false;
        }
        m_usleep = us;
    }
    return true;
}

MATCH_RESULT DatNodeRoot::Match(RRMessage* msg)
{
    if (! m_fmtNode)
    {
        return INNER_ERROR;
    }
    FmtNode* fn = msg->GetFmtNode();
    if (fn && m_fmtNode != fn)
    {
        GLOG(IM_DEBUG, "not the same fmt node");
        return UNMATCH_FMT;
    }
    //默认解析过，说明ok
    bool parseOk = true;

    //没有解析，需要先解析
    if (! msg->Parsed())
    {
        //对于性能测试，可以通过配置一个空的fmt.xml来达到加速效果
        if (msg->BlackFid(m_fid))
        {//如果在黑名单中，则肯定失败
            parseOk = false;
        }
        else 
        {//如果不在黑名单中就解析
            parseOk = m_fmtNode->Parse(msg);
            if (! parseOk)
            {//解析失败加黑名单
                msg->AddBlackFid(m_fid);
                GLOG(IM_DEBUG, "add %s to black fid", m_fid.c_str());
            }
            else
            {
                GLOG(IM_INFO, "[parse ok] id=%s\n------------\n%s.", m_debugId, msg->DebugString().c_str());
            }
        }
    }
    if (m_match & MATCH_FMT_OK)
    {// 要求是格式正确
        if (parseOk)
        {
            return MATCH_OK;
        }
        return UNMATCH_FMT;
    }
    if (m_match & MATCH_FMT_ERR)
    {//要求是格式错误
        if (parseOk)
        {
            return UNMATCH_FMT;
        }
        return MATCH_OK;
    }
    else if (! parseOk)
    {
        return UNMATCH_FMT;
    }
    EList<DatNode*>& v = msg->GetNode();
    for (DatNode** i = v.begin(); ! v.end(); i = v.next())
    {
        if (MATCH_OK != Match(*i, msg))
        {
            return UNMATCH_VALUE;
        }
    }
    return MATCH_OK;
}

int DatNodeRoot::GetRate()
{
    return m_rate;
}

bool DatNodeRoot::Serialize(RRMessage* dst, RRMessage* src, const string& fid)
{
    // 对repeated_num进行处理先
    ProcessDynamicRepeatedNode(src);
    string s; 
    EList<DatNode*> v;
    v.push_back(this);
    if (m_fmtNode->Serialize(dst, v, s, src))
    {
        GLOG(IM_DEBUG, "--q=%s:return :%s", m_debugId, Str::ToPrint(s).c_str());
        dst->SetBody(s);
        if (m_usleep > 0)
        {
            dst->SetUSleep(m_usleep);
        }
        return true;
    }
    return false;
}

const char* DatNodeRoot::GetDebugId()
{
    return m_debugId;
}

void DatNodeRoot::SetAttr(Node* n)
{
    DatNode::SetAttr(n);
    DatNodeRoot* dnr = (DatNodeRoot*)n;
    m_fmtNode = dnr->m_fmtNode;
    m_fid = dnr->m_fid;
    m_rate = dnr->m_rate;
    m_usleep = dnr->m_usleep;
    m_debugId = dnr->m_debugId;
}
