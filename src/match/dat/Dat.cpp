#include "Dat.h"
#include "Log.h"

using namespace std;

Dat::Dat() :
    m_doc(NULL)
{
}

Dat::~Dat()
{
    for (QA** it = m_vecQA.begin(); ! m_vecQA.end(); it = m_vecQA.next())
    {
        delete *it;
    }
    m_vecQA.clear();
    delete m_doc;
}

bool Dat::Load(const char* datFile, Fmt* fmt, 
        const string& qfid, const string& afid)
{
    delete m_doc;
    m_doc = new TiXmlDocument;
    m_doc->LoadFile(datFile);
    if (m_doc->Error())
    {
        GLOG(IM_ERROR, "load %s failed:%s line:%d, col:%d", 
                datFile, m_doc->ErrorDesc(),
                m_doc->ErrorRow(),
                m_doc->ErrorCol());
        return false;
    }
    TiXmlHandle hDoc(m_doc);
    TiXmlElement* root;
    root = hDoc.FirstChildElement("dat").Element();
    if (! root)
    {
        GLOG(IM_ERROR, "root element isn't \"dat\" in %s", datFile);
        return false;
    }
    int n = 1;
    for (TiXmlElement* c = root->FirstChildElement("qa");
            c; c = c->NextSiblingElement("qa"), ++n)
    {
        QA* qa = new QA();
        if (! qa->Load(c, fmt, qfid, afid))
        {
            GLOG(IM_ERROR, "load qa NO.%d error", n);
            delete qa;
            return false;
        }
        m_vecQA.push_back(qa);
    }
    return true;
}

QA* Dat::Match(RRMessage* msg, bool isQ, const string& fid)
{
    int n = 1;
    for (QA** it = m_vecQA.begin(); ! m_vecQA.end(); it = m_vecQA.next(), ++n)
    {
        GLOG(IM_DEBUG, "begin check qa NO.%d match", n);
        if ((*it)->Match(msg, isQ, fid))
        {
            GLOG(IM_DEBUG, "qa NO.%d match ok", n);
            return (*it);
        }
        GLOG(IM_DEBUG, "qa NO.%d match failed", n);
    }
    GLOG(IM_INFO, "[match failed] no qa match");
    return NULL;
}

EList<QA*>& Dat::GetQA()
{
    return m_vecQA;
}
