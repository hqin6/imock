#include "Fmt.h"
#include "Log.h"
#include "FmtNodeFactory.h"

using namespace std;

Fmt::Fmt() : m_doc(NULL)
{
}

Fmt::~Fmt()
{
    for (map<string, Node*>::iterator it = m_mp_fmtQ.begin();
            it != m_mp_fmtQ.end(); ++it)
    {
        delete it->second;
    }

    for (map<string, Node*>::iterator it = m_mp_fmtA.begin();
            it != m_mp_fmtA.end(); ++it)
    {
        delete it->second;
    }
    delete m_doc;
}

bool Fmt::Load(const char* fmtFile)
{
    delete m_doc;
    m_doc = new TiXmlDocument;
    m_doc->LoadFile(fmtFile);
    if (m_doc->Error())
    {
        GLOG(IM_ERROR, "load %s failed:%s line:%d, col:%d", 
                fmtFile, m_doc->ErrorDesc(),
                m_doc->ErrorRow(),
                m_doc->ErrorCol());
        return false;
    }
    TiXmlHandle hDoc(m_doc);
    TiXmlElement* root;
    root = hDoc.FirstChildElement("fmt").Element();
    if (! root)
    {
        GLOG(IM_ERROR, "root element isn't \"fmt\" in %s", fmtFile);
        return false;
    }
    struct {
        const char* key;
        map<string, Node*>* fmtMap;
    } *tmp, mp[] = {
        {"q", &m_mp_fmtQ }, //query节点，支持多个
        {"a", &m_mp_fmtA }, //answer节点，支持多个
        {NULL,NULL       }
    };
    Node::SetFactory(FmtNodeFactory::Get);
    for (tmp = mp; tmp->key; ++tmp)
    {
        for (TiXmlElement* c = root->FirstChildElement(tmp->key);
                c; c = c->NextSiblingElement(tmp->key))
        {
            const char* fid = c->Attribute("fid");
            if (! fid)
            {
                fid = "";
            }
            if (tmp->fmtMap->end() != tmp->fmtMap->find(fid))
            {
                GLOG(IM_ERROR, "fid \"%s\" repeat in %s", fid, fmtFile);
                Node::SetFactory(NULL);
                return false;
            }
            Node* n = FmtNodeFactory::Get(c);
            if (!n || ! n->Load(c))
            {
                delete n;
                Node::SetFactory(NULL);
                return false;
            }
            (*(tmp->fmtMap))[fid] = n;
        }
        if (tmp->fmtMap->empty())
        {
            GLOG(IM_ERROR, "no element \"%s\" in %s", tmp->key, fmtFile);
            Node::SetFactory(NULL);
            return false;
        }
    }
    Node::SetFactory(NULL);
    return true;
}

map<string, Node*>* Fmt::GetQ()
{
    return &m_mp_fmtQ;
}
map<string, Node*>* Fmt::GetA()
{
    return &m_mp_fmtA;
}

