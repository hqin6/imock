#include <fstream>
#include <sstream>
#include "DiffInfo.h"
#include "INIConf.h"
#include "Log.h"

using namespace std;

DiffInfo::DiffInfo(const string& name) : m_name(name)
{
}

DiffInfo::~DiffInfo()
{
}

const char* DiffInfo::GetName()
{
    return m_name.c_str();
}

bool DiffInfo::Init()
{
    if (! g_iniConf)
    {
        GLOG(IM_ERROR, "no INI configure");
        return false;
    }
    Command c[] = {
        { m_name.c_str(), "file", ISet::Str,  (uint64_t)&m_file, NULL  },
        { m_name.c_str(), "format",  ISet::Str,  (uint64_t)&m_fmtFile, NULL },
        { NULL,   NULL,      NULL,       0,      NULL  }
    };
    if (! g_iniConf->Get(c, 0))
    {
        return false;
    }
    return true;
}

bool DiffInfo::ParseFile(DatNode* attrRoot, bool isq)
{
    // load fmt
    if (! m_fmt.Load(m_fmtFile.c_str()))
    {
        return false;
    }

    // load file
    ifstream in(m_file.c_str());
    ostringstream ss;
    ss << in.rdbuf();
    m_msg.SetBody(ss.str());
    map<string, Node*>* fmt = NULL;
    if (isq)
    {
        fmt = m_fmt.GetQ();
    }
    else
    {
        fmt = m_fmt.GetA();
    }
    // parse
    map<string, Node*>::iterator it = fmt->begin();
    for (; it != fmt->end(); ++it)
    {
        FmtNode* fn = (FmtNode*)it->second;
        m_msg.SetCurFid(it->first);
        if (fn->Parse(&m_msg))
        {//解析成功
            break;
        }
    }
    if (fmt->end() == it)
    {
        GLOG(IM_ERROR, "Parse %s failed", m_file.c_str());
        return false;
    }
    GLOG(IM_DEBUG, "Parse %s ok", m_file.c_str());
    // 获取_im_diff_key
    EList<DatNode*>& d = m_msg.GetNode();
    DatNode** n;
    for (n = d.begin(); ! d.end(); n = d.next())
    {
        DatNode* p = (DatNode*)(*n)->GetSub("_im_diff_key");
        if (! p)
        {
            GLOG(IM_WARN, "no _im_diff_key , use all match");
            break;
        }
        m_keyNodes[p->GetValue()] = (DatNodeRoot*)MergeValueAttr(*n, attrRoot);
    }
    if (! d.end() && ! m_keyNodes.empty())
    {
        GLOG(IM_ERROR, "oh no, impossible! contact me!");
        return false;
    }
    return true;
}

DatNode* DiffInfo::MergeValueAttr(DatNode* s1, DatNode* s2)
{
    // 取s1的值和s2的属性
    DatNode* d = new DatNode;
    d->SetName(s1->GetName());
    d->SetValue(s1->GetValue());
    d->SetAttr(s2);
    // 对所有的s1的sub节点,递归
    EList<Node*>& en = s1->GetSubs();
    Node** p1 = en.begin();
    for ( ; ! en.end(); p1 = en.next())
    {
        // 获取s2同名的节点
        DatNode* p2 = (DatNode*)s2->GetSub((*p1)->GetName());
        if (p2)
        {
            DatNode* c = MergeValueAttr((DatNode*)*p1, p2);
            d->AddChild(c);
        }
    }
    return d;
}

bool DiffInfo::DoDiff(DiffInfo* di, bool brk, string& resultStr)
{
    if (0 == Equal(di, brk, resultStr) 
            && 0 == di->Equal(this, brk, resultStr))
    {
        return true;
    }
    return false;
}

static void AddMatchResult(string& resultStr, 
        bool m, const string& a1, 
        const string& a2, const string& k)
{
    if (m)
    {
        resultStr += "[yes]";
    }
    else 
    {
        resultStr += "[no ]";
    }
    resultStr += " " + a1 + " vs " + a2 + " : " + k + "\n";
    return ;
}

int DiffInfo::Equal(DiffInfo* di, bool brk, string& resultStr)
{
    int noMatchNum = 0;
    if (m_keyNodes.empty())
    {// 全量比较
        EList<DatNode*>& m1 = m_msg.GetNode();
        EList<DatNode*>& m2 = di->m_msg.GetNode();
        DatNode** p1, ** p2;
        for (p1 = m1.begin(); ! m1.end(); p1 = m1.next())
        {
            for (p2 = m2.begin(); ! m2.end(); p2 = m2.next())
            {
                if (MATCH_OK == (*p1)->Match(*p2, &di->m_msg))
                {
                    break;
                }
            }
            string tmp = (*p1)->DebugXmlString(0);
            if (m2.end())
            {
                noMatchNum++;
                GLOG(IM_ERROR, "can't find matched for %s:%s in %s",
                        m_name.c_str(),
                        tmp.c_str(), 
                        di->m_name.c_str());
                AddMatchResult(resultStr, false, m_name,
                        di->m_name, tmp);
                if (brk)
                {// 终止
                    break;
                }
            }
            else
            {
                GLOG(IM_INFO, "equal %s = %s for %s",
                        m_name.c_str(),
                        di->m_name.c_str(),
                        tmp.c_str());
                AddMatchResult(resultStr, true, m_name,
                        di->m_name, tmp);
            }
        }
    }
    else
    {// 按key值比较
        map<string, DatNodeRoot*>::iterator i1;
        map<string, DatNodeRoot*>::iterator i2;

        for (i1 = m_keyNodes.begin(); i1 != m_keyNodes.end();
                ++i1)
        {
            i2 = di->m_keyNodes.find(i1->first);
            if (i2 == di->m_keyNodes.end())
            {
                noMatchNum++;
                GLOG(IM_ERROR, "can't find matched for %s:%s in %s",
                        m_name.c_str(),
                        i1->first.c_str(), 
                        di->m_name.c_str());
                if (brk)
                {// 终止
                    break;
                }
                AddMatchResult(resultStr, false, m_name,
                        di->m_name, i1->first);
            }
            else 
            {
                if (MATCH_OK == i1->second->Match(i2->second, &di->m_msg))
                {
                    GLOG(IM_INFO, "equal %s = %s for %s",
                            m_name.c_str(),
                            di->m_name.c_str(),
                            i1->first.c_str());
                    AddMatchResult(resultStr, true, m_name,
                            di->m_name, i1->first);
                }
                else
                {
                    GLOG(IM_ERROR, "no equal %s != %s for %s",
                            m_name.c_str(),
                            di->m_name.c_str(),
                            i1->first.c_str());
                    AddMatchResult(resultStr, false, m_name,
                            di->m_name, i1->first);
                }
            }
        }
    }
    if (noMatchNum)
    {
        GLOG(IM_INFO, "%s vs %s : found %d different(s)",
                m_name.c_str(),
                di->m_name.c_str(),
                noMatchNum);
    }
    else
    {
        GLOG(IM_INFO, "%s vs %s : equal",
                m_name.c_str(),
                di->m_name.c_str()
                );
    }
    return noMatchNum;
}
