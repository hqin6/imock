#include "imock-interface.h"

#include "Log.h"
#include "Fmt.h"
#include "Dat.h"

using namespace std;
using namespace log4cpp;

Logger* g_log = NULL;

ImockInterface::ImockInterface()
{
    m_fmt = NULL;
    m_dat = NULL;
}

ImockInterface::~ImockInterface()
{
    delete (Fmt*)m_fmt;
    delete (Dat*)m_dat;
}

bool ImockInterface::Init(const string& logLevel)
{
    Priority::Value v;
    {
        map<string, Priority::Value> mp;
        mp["emerg" ] = IM_EMERG;
        mp["fatal" ] = IM_FATAL;
        mp["alert" ] = IM_ALERT;
        mp["crit"  ] = IM_CRIT;
        mp["error" ] = IM_ERROR;
        mp["warn"  ] = IM_WARN;
        mp["notice"] = IM_NOTICE;
        mp["info"  ] = IM_INFO;
        mp["debug" ] = IM_DEBUG;
        map<string, Priority::Value>::iterator it = mp.find(logLevel);
        if (mp.end() == it)
        {
            printf("unrecognized log level:%s\n", logLevel.c_str());
            return false;
        }
        v = it->second;
    }

    g_log = new Logger("/dev/stdout");
    g_log->SetLevel(v);
    if (! g_log->Load())
    {
        return false;
    }
    return true;
}

bool ImockInterface::LoadFmtFile(const string& fmtFile)
{
    if (m_fmt)
    {
        delete (Fmt*)m_fmt;
        m_fmt = NULL;
    }
    Fmt* fmt = new Fmt;
    m_fmt = fmt;

    if (! fmt->Load(fmtFile.c_str()))
    {
        return false;
    }
    return true;
}

bool ImockInterface::LoadDatFile(const string& datFile, const string& qfid, const string& afid)
{
    if (! m_fmt)
    {
        return false;
    }
    if (m_dat)
    {
        delete (Dat*)m_dat;
        m_dat = NULL;
    }
    Dat* dat = new Dat;
    m_dat = dat;

    if (! dat->Load(datFile.c_str(), (Fmt*)m_fmt, qfid, afid))
    {
        return false;
    }
    return true;
}

bool ImockInterface::Parse(const string& src,
        string& xml, bool isq, const string& fid, void** pobj,
        const string& sepb, const string& sepe)
{
    // 构造message
    RRMessage* req = new RRMessage;
    if (pobj)
    {
        *pobj = req;
    }
    req->SetBody(src);

    map<string, Node*>* fmt = NULL;
    if (isq) 
    {
        fmt = ((Fmt*)m_fmt)->GetQ();
    }
    else
    {
        fmt = ((Fmt*)m_fmt)->GetA();
    }
    for (map<string,Node*>::iterator it = fmt->begin();
            it != fmt->end(); ++it)
    {
        if (! fid.empty())
        {
            if (fid != it->first)
            {
                continue;
            }
        }
        FmtNode* fn = (FmtNode*)it->second;
        req->SetCurFid(it->first);
        if (fn->Parse(req))
        {//解析成功
            xml = req->DebugXmlString(sepb, sepe);
            return true;
        }
    }
    return false;
}

bool ImockInterface::PMatch(const string& src, string& xml, bool isq, const string& fid, void** pobj, void** cobj)
{
    Dat* dat = (Dat*)m_dat;
    if (! dat)
    {
        return false;
    }
    // 构造message
    RRMessage* req = new RRMessage;
    if (pobj)
    {
        *pobj = req;
    }
    req->SetBody(src);

    QA* qa = dat->Match(req, isq, fid);
    xml = req->DebugXmlString();
    if (cobj)
    {
        *cobj = qa;
    }
    return qa != NULL;
}

bool ImockInterface::Serialize(void* r, vector<string>& v, const string& fid)
{
    vector<DatNodeRoot*>* root = (vector<DatNodeRoot*>*)r;
    RRMessage req;
    for (vector<DatNodeRoot*>::iterator i = root->begin(); i != root->end(); ++i)
    {
        if ((*i)->Serialize(&req, NULL, fid))
        {
            v.push_back(req.GetBody());
        }
        else
        {
            printf("serialize failed.\n");
            return false;
        }
    }
    return true;
}

bool ImockInterface::Serialize(vector<IQA>& dst)
{
    Dat* dat = (Dat*)m_dat;

    if (! dat)
    {
        return false;
    }

    // 遍历所有的QA节点
    EList<QA*>& v = dat->GetQA();
    for (QA** it = v.begin(); ! v.end(); it = v.next())
    {
        IQA iqa;
        if (! Serialize(&(*it)->GetQ(), iqa.q, iqa.qfid) || ! Serialize(&(*it)->GetA(), iqa.a, iqa.afid))
        {
            return false;
        }
        dst.push_back(iqa);
    }
    return true;
}

bool ImockInterface::Match(void* pobj1, void* cobj2, bool isq, const string& fid)
{
    RRMessage* msg = (RRMessage*)pobj1;
    QA* qa = (QA*)cobj2;
    return qa->Match(msg, isq, fid);
}

void ImockInterface::Delete(void* obj)
{
    delete (RRMessage*)obj;
}
