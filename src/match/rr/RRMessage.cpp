#include "RRMessage.h"
#include "DatNode.h"
#include "Log.h"
using namespace std;

RRMessage::RRMessage() :
    m_usleep(0),
    m_httpVersionMinor(-1),
    m_hasFailedMsg(false)
{
}

RRMessage::~RRMessage()
{
    for (map<string, EList<DatNode*>*>::iterator mit = m_nodes.begin();
            mit != m_nodes.end(); ++mit)
    {
        for (DatNode** it = mit->second->begin(); 
            ! mit->second->end(); it = mit->second->next())
        {
            delete *it;
        }
        delete mit->second;
    }
}

void RRMessage::SetCurFid(const string& fid)
{
    m_curFid = fid;
}

bool RRMessage::Parsed()
{
    return m_nodes.find(m_curFid) != m_nodes.end();
}

bool RRMessage::BlackFid(const string& fid)
{
    return m_blackFid.find(fid) != m_blackFid.end();
}

void RRMessage::AddBlackFid(const string& fid)
{
    m_blackFid.insert(fid);
}

EList<DatNode*>& RRMessage::GetNode()
{
    return *m_nodes[m_curFid];
}

void RRMessage::AddNode(DatNode* root)
{
    map<string, EList<DatNode*>* >::iterator it = m_nodes.find(m_curFid);
    EList<DatNode*>* p = NULL;
    if (it != m_nodes.end())
    {
        p = it->second;
    }
    else
    {
        p = new EList<DatNode*>;
        m_nodes.insert(pair<string, EList<DatNode*>*>(m_curFid, p));
    }
    p->push_back(root);
}

const char* RRMessage::GetVar(const string& v, bool& found)
{
    found = false;
    map<string, map<string, string> >::iterator mid = m_vars.find(m_curFid);
    if (mid == m_vars.end())
    {
        GLOG(IM_DEBUG, "no fid=%s for var<%s>", m_curFid.c_str(), v.c_str());
        return "";
    }
    map<string, string>::iterator it;
    it = mid->second.find(v);
    if (mid->second.end() == it)
    {
        GLOG(IM_DEBUG, "get var<%s=\"\">", v.c_str());
        return "";
    }
    GLOG(IM_DEBUG, "get var<%s=%s>", v.c_str(), it->second.c_str());
    found = true;
    return (it->second.c_str());
}

const string& RRMessage::GetFailedMsg()
{
    return m_failedMsg;
}

void RRMessage::SetFailedMsg(const string& msg)
{
    m_hasFailedMsg = true;
    m_failedMsg = msg;
}

bool RRMessage::HasFailedMsg()
{
    return m_hasFailedMsg;
}

const string& RRMessage::GetMethod()
{
    return m_method;
}

const string& RRMessage::GetService()
{
    return m_service;
}

const string& RRMessage::GetUriPath()
{
    return m_uriPath;
}

const string& RRMessage::GetUriQuery()
{
    return m_uriQuery;
}

const string& RRMessage::GetHeader()
{
    return m_header;
}

void RRMessage::ClearVars()
{
    m_vars.erase(m_curFid);
}

void RRMessage::AddVar(const string& k, const string& v)
{
    GLOG(IM_DEBUG, "add var<%s=%s> for fid=%s", 
            k.c_str(), v.c_str(), m_curFid.c_str());
    m_vars[m_curFid][k] = v;
}

void RRMessage::DelVar(const string& k)
{
    GLOG(IM_DEBUG, "del var<%s> for fid=%s", 
            k.c_str(), m_curFid.c_str());

    m_vars[m_curFid][k] = "";
}

void RRMessage::SetFmtNode(FmtNode* n)
{
    m_fmtNodes[m_curFid] = n;
}

FmtNode* RRMessage::GetFmtNode()
{
    return m_fmtNodes[m_curFid];
}

string RRMessage::DebugString()
{
    string s;
    map<string, EList<DatNode*>* >::iterator mit = m_nodes.find(m_curFid);
    if (mit == m_nodes.end())
    {
        return "";
    }
    for (DatNode** it = mit->second->begin(); 
        ! mit->second->end(); it = mit->second->next())
    {
        s += (*it)->DebugString(0);
    }
    return s;
}

string RRMessage::DebugXmlString(const string& sepb,
        const string& sepe)
{
    string s;
    map<string, EList<DatNode*>* >::iterator mit = m_nodes.find(m_curFid);
    if (mit == m_nodes.end())
    {
        return "";
    }
    for (DatNode** it = mit->second->begin(); 
        ! mit->second->end(); it = mit->second->next())
    {
        s += sepb + (*it)->DebugXmlString(0) + sepe;
    }
    return s;
}

void RRMessage::SetMethod(const string& s)
{
    m_method = s;
}

void RRMessage::SetService(const string& s)
{
    m_service = s;
}

void RRMessage::SetUriPath(const string& s)
{
    m_uriPath = s;
}

void RRMessage::SetUriQuery(const string& s)
{
    m_uriQuery = s;
}

void RRMessage::SetHeader(const string& s)
{
    m_header = s;
}

void RRMessage::SetHttpVersionMinor(int m)
{
    m_httpVersionMinor = m;
}

int RRMessage::GetHttpVersionMinor()
{
    return m_httpVersionMinor;
}

void RRMessage::SetBody(const string& b)
{
    m_body = b;
}

const string& RRMessage::GetBody()
{
    return m_body;
}

void RRMessage::SetUSleep(int usleep)
{
    m_usleep = usleep;
}

int RRMessage::GetUSleep()
{
    return m_usleep;
}

void RRMessage::AddHttpHead(const string& k, const string& v)
{
    m_httpHead.push_back(make_pair(k, v));
}

void RRMessage::SetHttpCode(const string& c)
{
    m_httpCode = c;
}

void RRMessage::SetHttpCodeMsg(const string& msg)
{
    m_httpCodeMsg = msg;
}

EList<pair<string, string> >& RRMessage::GetHttpHead()
{
    return m_httpHead;
}

const string& RRMessage::GetHttpCode()
{
    return m_httpCode;
}

const string& RRMessage::GetHttpCodeMsg()
{
    return m_httpCodeMsg;
}

void RRMessage::AddHttpUriQuery(const string& k, const string& v)
{
    m_httpUriQuery.push_back(make_pair(k, v));
}

EList<pair<string, string> >& RRMessage::GetHttpUriQuery()
{
    return m_httpUriQuery;
}
