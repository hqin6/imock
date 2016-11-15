#include "QA.h"
#include "Log.h"
#include "DatNodeFactory.h"
using namespace std;

QA::QA() : m_debugId("")
{
}

QA::~QA()
{
    for (vector<DatNodeRoot*>::iterator it = m_vecQ.begin();
            it != m_vecQ.end(); ++it)
    {
        delete *it;
    }
    for (vector<DatNodeRoot*>::iterator it = m_vecA.begin();
            it != m_vecA.end(); ++it)
    {
        delete *it;
    }
}

bool QA::Load(TiXmlElement* e, Fmt* fmt, const string& qfid, const string& afid)
{
    const char* p = e->Attribute("id");
    if (p)
    {
        m_debugId = p;
    }
    struct {
        const char* key;
        vector<DatNodeRoot*>* vec;
        map<string, Node*>* mp;
        string fid;
    } *tmp, v[] = {
        {"q", &m_vecQ, fmt ? fmt->GetQ() : NULL, qfid },
        {"a", &m_vecA, fmt ? fmt->GetA() : NULL, afid }, 
        {NULL, NULL, NULL}
    };
    Node::SetFactory(DatNodeFactory::Get);
    for (tmp = v; tmp->key; ++tmp)
    {
        int n = 1;
        for (TiXmlElement* c = e->FirstChildElement(tmp->key);
                c; c = c->NextSiblingElement(tmp->key), ++n)
        {
            DatNodeRoot* dn = new DatNodeRoot();
            if (!dn->LoadAttr(c, tmp->mp, tmp->fid) || !dn->Load(c))
            {
                GLOG(IM_ERROR, "load %s NO.%d failed.", tmp->key, n);
                delete dn;
                Node::SetFactory(NULL);
                return false;
            }
            tmp->vec->push_back(dn);
        }
    }
    Node::SetFactory(NULL);
    return true;
}

bool QA::Match(RRMessage* msg, bool isQ, const string& fid)
{
    int n = 1;
    vector<DatNodeRoot*>* vec = &m_vecA;
    if (isQ)
    {
        vec = &m_vecQ;
    }
    for (vector<DatNodeRoot*>::iterator it = vec->begin();
            it != vec->end(); ++it, ++n)
    {
        GLOG(IM_DEBUG, "[match check] qa:%s, NO.%d, %c:%s", 
                m_debugId, n, isQ ? 'q' : 'a', (*it)->GetDebugId());
        if (! fid.empty())
        {
            if (fid != (*it)->GetFid())
            {
                GLOG(IM_DEBUG, "skip %s, fid != %s", (*it)->GetFid().c_str(), fid.c_str());
                continue;
            }
        }
        msg->SetCurFid((*it)->GetFid());
        if (MATCH_OK == (*it)->Match(msg))
        {//match上
            GLOG(IM_INFO, "[match ok] qa:%s, NO.%d, %c:%s\n"
                    "------match------\n%s",
                    m_debugId, n, isQ ? 'q' : 'a',
                    (*it)->GetDebugId(), (*it)->DebugString(0).c_str());
            return true;
        }
        GLOG(IM_DEBUG, "[match failed] qa:%s, NO.%d, %c:%s",
                m_debugId, n, 
                isQ ? 'q' : 'a', 
                (*it)->GetDebugId());
    }
    GLOG(IM_DEBUG, "[match failed] no match %c in qa:%s", 
            isQ ? 'q' : 'a',
            m_debugId);
    return false;
}

bool QA::Query(RRMessage* dst, const string& qID)
{
    DatNodeRoot* d = GetByRate(m_vecQ, qID);
    if (NULL == d)
    {
        return false;
    }
    GLOG(IM_INFO, "[choice] qa:%s, q:%s\n------query------\n%s",
            m_debugId, d->GetDebugId(), d->DebugString(0).c_str());
    return d->Serialize(dst);
}

bool QA::Answer(RRMessage* dst, RRMessage* src)
{
    DatNodeRoot* d = GetByRate(m_vecA);
    if (NULL == d)
    {
        return false;
    }
    GLOG(IM_INFO, "[choice] qa:%s, a:%s\n------answer------\n%s", 
            m_debugId, d->GetDebugId(), d->DebugString(0).c_str());
    return d->Serialize(dst, src);
}

DatNodeRoot* QA::GetByRate(vector<DatNodeRoot*>& vec, const string& qID)
{
    int total = 0;
    int d = random();
    for (vector<DatNodeRoot*>::iterator it = vec.begin();
            it != vec.end(); ++it)
    {
        if (! qID.empty())
        {
            if (qID != (*it)->GetDebugId())
            {
                GLOG(IM_DEBUG, "skip %s, qid != %s.", (*it)->GetDebugId(), qID.c_str());
                continue;
            }
        }
        int r = (*it)->GetRate();
        if (r < 0)
        {
            return vec[d % vec.size()];
        }
        total += (*it)->GetRate();
    }
    if (0 == total)
    {
        return NULL;
    }
    int sum = 0;
    int idx = d % total;
    for (vector<DatNodeRoot*>::iterator it = vec.begin();
            it != vec.end(); ++it)
    {
        int r = (*it)->GetRate();
        if (0 == r)
        {
            continue;
        }
        sum += r;
        if (idx < sum)
        {
            return *it;
        }
    }
    return NULL;
}

vector<DatNodeRoot*>& QA::GetQ()
{
    return m_vecQ;
}

vector<DatNodeRoot*>& QA::GetA()
{
    return m_vecA;
}

const char* QA::GetDebugID()
{
    return m_debugId;
}
