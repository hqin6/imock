#include <sys/time.h>
#include <map>
#include <algorithm>
#include <fstream>
#include "DatNode.h"
#include "Log.h"
#include "Str.h"
#include "File.h"
#include "System.h"
#include "Zlib.h"
#include "Var.h"
using namespace std;

DatNode::DatNode() : 
    m_from(0),
    m_match(0),
    m_reg(NULL)
{
}

DatNode::~DatNode()
{
    delete m_reg;
}

static int s_fullNameMaxLen = 0;

MATCH_RESULT DatNode::Match(DatNode* n, RRMessage* msg)
{//n 是request数据
    //比较字段名字
    if (n->m_name != m_name)
    {
        GLOG(IM_DEBUG, "check key: %s != %s", 
                n->m_name.c_str(), m_name.c_str());
        return UNMATCH_NAME;
    }
    GLOG(IM_DEBUG, "check key: %s=%s", 
            n->m_name.c_str(), m_name.c_str());
    /*if (m_subs.empty() && ! n->m_subs.empty())
    {//dat中为空，但是请求不为空，则不符合
        GLOG(IM_DEBUG, "check subs num failed");
        return false;
    }*/
    //如果没有子字段，比较本字段
    if (0 == m_subs.size())
    {
        return MatchValue(n, msg);
    }
    //如果有子字段，比较子字段
    for (Node** i = m_subs.begin(); ! m_subs.end(); i = m_subs.next())
    {
        Node** j = n->m_subs.begin();
        GLOG(IM_DEBUG, "check sub key:%s begin", (*i)->GetName().c_str());
        bool exist = false;
        for (; ! n->m_subs.end(); j = n->m_subs.next())
        {
            MATCH_RESULT mr = ((DatNode*)*i)->Match((DatNode*)*j, msg);
            if (MATCH_OK == mr)
            {//有任何一个匹配成功，即可
                break;
            }
            else if (UNMATCH_NAME != mr)
            {
                exist = true;
            }
        }
        if (n->m_subs.end())
        {//没有匹配成功
            if ((! exist) && (((DatNode*)*i)->m_match & MATCH_NO_EXIST))
            {
                GLOG(IM_INFO, "[match(%04X)] [%s] [%-*s] [dat=%s] [r=%s]",
                        ((DatNode*)*i)->m_match, "succ", 
                        s_fullNameMaxLen, (*i)->GetFullName().c_str(), 
                        "no exist", "no exist");
                continue;
            }
            else if (exist && (((DatNode*)*i)->m_match & MATCH_EXIST))
            {
                GLOG(IM_INFO, "[match(%04X)] [%s] [%-*s] [dat=%s] [r=%s]",
                        ((DatNode*)*i)->m_match, "succ", 
                        s_fullNameMaxLen, (*i)->GetFullName().c_str(), 
                        "exist", "exist");
                continue;
            }
            else 
            {
                GLOG(IM_DEBUG, "check sub key:%s end, failed", 
                        (*i)->GetName().c_str());
                return UNMATCH_VALUE;
            }
        }
        else
        {// 匹配成功
            if (((DatNode*)*i)->m_match & MATCH_NO_EXIST)
            {// 要求不存在
                GLOG(IM_INFO, "[match(%04X)] [%s] [%-*s] [dat=%s] [r=%s]",
                        ((DatNode*)*i)->m_match, "fail", 
                        s_fullNameMaxLen, (*i)->GetFullName().c_str(), 
                        "no_exist", "exist");
                return UNMATCH_VALUE;
            }
        }
    }
    return MATCH_OK;
}

MATCH_RESULT DatNode::MatchValue(DatNode* n, RRMessage* msg)
{
    //n是request
    //v1里可能有正则
    string v2 = n->GetValue(msg);
    string v1 = GetValue(msg, v2);
    if (m_match & MATCH_ICASE)
    {
        transform(v1.begin(), v1.end(), v1.begin(), ::tolower);
        transform(v2.begin(), v2.end(), v2.begin(), ::tolower);
    }
    bool m = false;
    if (m_match & MATCH_REG)
    {
        if (! m_reg || m_reg->GetPattern() != v1)
        {
            delete m_reg;
            GLOG(IM_DEBUG, "new reg:%s", v1.c_str());
            m_reg = new Reg(v1);
            if (! m_reg->Init())
            {
                delete m_reg;
                m_reg = NULL;
                return INNER_ERROR;
            }
        }
        m = m_reg->Match(v2);
    }
    else if (m_match & MATCH_LT)
    {
        m = ValueCmp(v2, v1) < 0;
    }
    else if (m_match & MATCH_EQ)
    {
        m = ValueCmp(v1, v2) == 0;
    }
    else if (m_match & MATCH_NE)
    {
        m = ValueCmp(v1, v2) != 0;
    }
    else if (m_match & MATCH_GT)
    {
        m = ValueCmp(v2, v1) > 0;
    }
    else if (m_match & MATCH_SHELL)
    {
        vector<string> shellArgs;
        ShellArgs(m_matchShellCmdArr, m_match & MATCH_SHELL_RVAR, msg, shellArgs);
        System cmd;
        for (vector<string>::iterator it = shellArgs.begin();
                it != shellArgs.end(); ++it)
        {
            cmd.Add(it->c_str());
        }
        cmd.Add(v1.c_str());
        cmd.Add(v2.c_str());
        string v = cmd.Cmd();
        m = (! v.empty() && v[0] == '0');
    }
    else 
    {
        m = v1 == v2;
    }
    //GLOG(IM_INFO, "(%s) match(%04X):%s<=>%s", 
            //m ? "true" : "false", m_match, v1.c_str(), v2.c_str());
    string tmp = "";
    GLOG(IM_INFO, "[match(%04X)] [%s] [%-*s] [dat=%s] [r=%s]",
            m_match, m ? "succ" : "fail", 
            s_fullNameMaxLen, (tmp = GetFullName()).c_str(), 
            v1.c_str(), v2.c_str());

    s_fullNameMaxLen = s_fullNameMaxLen > (int)tmp.length() 
        ? s_fullNameMaxLen : tmp.length();
    return m == true ? MATCH_OK : UNMATCH_VALUE;
}

const string DatNode::GetValue(RRMessage* msg, const string& rv)
{
    string res;
    res = m_value;
    if (m_from & FROM_FILE)
    {//需要从文件里读取
        // 获取文件名
        string fileName = m_value;
        if (! m_value.empty() && m_value[0] == '$')
        {// 文件名有可能是变量
            bool found = false;
            fileName = msg->GetVar(m_value, found);
        }
        bool cached = m_from & FILE_CACHED;
        if (cached)
        {//可以缓存文件
            map<string, string>::iterator it;
            it = s_fileCache.find(fileName);
            if (it != s_fileCache.end())
            {
                res = it->second;
                goto OK;
            }
        }
        //读取文件
        string content = "";
        ifstream ifs(fileName.c_str());
        if (ifs.good())
        {
            stringstream ss;
            ss << ifs.rdbuf();
            content = ss.str();
        }
        if (cached)
        {
            s_fileCache[fileName] = content;
        }
        res = content;
        goto OK;
    }
    /*else if (m_from & FROM_SHELL)
    {// 需要从shell里读取
        bool cached = m_from & FILE_CACHED;
        if (cached)
        {//可以缓存
            map<string, string>::iterator it;
            it = s_shellCache.find(m_value);
            if (it != s_shellCache.end())
            {
                res = it->second;
                goto OK;
            }
        }
        // 传递value body head
        string  head = "", body = "", rvalue = "";
        string prefix = "/tmp/.imock.d.";
        string suffix = "";
        if (m_from & FROM_SHELL_HEAD 
                || m_from & FROM_SHELL_BODY 
                || m_from & FROM_SHELL_RVALUE)
        {// 构造文件后缀
            struct timeval rtime;
            gettimeofday(&rtime, NULL);
            char buf[64];
            snprintf(buf, sizeof(buf), ".%d.%d", (int)getpid(), (int)rtime.tv_usec);
            suffix = buf;
        }
        if (m_from & FROM_SHELL_HEAD)
        {
            head = prefix + "head" + suffix;
            File::Write(head, msg->GetHeader());
        }
        if (m_from & FROM_SHELL_BODY)
        {
            body = prefix + "body" + suffix;
            File::Write(body, msg->GetBody());
        }
        if (m_from & FROM_SHELL_RVALUE)
        {
            rvalue = prefix + "rvalue" + suffix;
            File::Write(rvalue, rv);
        }
        // 替换变量
        string shellCmd = m_value;
        if (m_from & FROM_SHELL_RVAR)
        {
            Var m(shellCmd);
            string varName, o;
            varName = m.Next(o);
            while (varName != "")
            {
                bool found = false;
                string varVal = msg->GetVar(varName, found);
                m.Replace(0, o.size(), found ? &varVal : NULL);
                if (found)
                {
                    GLOG(IM_DEBUG, "replace var in cmd:%s to %s", 
                            varName.c_str(), varVal.c_str());
                }
                varName = m.Next(o);
            }
            m.ReplaceEnd();
            shellCmd = m.GetResult();
        }


        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "%s %s %s %s",
                shellCmd.c_str(), rvalue.c_str(),
                head.c_str(), body.c_str());
        res = System::Cmd(cmd);
        if (cached)
        {
            s_shellCache[m_value] = res;
        }
        if (! head.empty() || ! body.empty() || ! rvalue.empty())
        {// 删除
            snprintf(cmd, sizeof(cmd), "rm -f %s %s %s",
                    head.c_str(), body.c_str(), rvalue.c_str());
            system(cmd);
        }
        goto OK;
    }*/
    else if (msg && m_from & FROM_VAR)
    {
        bool found = false;
        res = msg->GetVar(m_value, found);
        goto OK;
    }
OK:
    ExecOP(res, msg, rv);
    GLOG(IM_DEBUG, "value =  %s, key = %s", Str::ToPrint(res).c_str(), m_name.c_str());
    return res;
}

void DatNode::SetValue(const string& v)
{
    m_value = v;
}

int DatNode::ValueCmp(const string& v1, const string& v2)
{
    if (m_match & MATCH_INT)
    {
        int i1, i2;
        i1 = atoi(v1.c_str());
        i2 = atoi(v2.c_str());
        return i1 - i2;
    }
    else
    {
        return v1.compare(v2);
    }
}

map<string, string> DatNode::s_fileCache;

bool DatNode::LoadAttr(TiXmlElement* e)
{
    const char* p = NULL;
    p = e->GetText();
    if (p)
    {
        m_value = p;
    }
    p = e->Attribute("match");
    if (p)
    {
        EList<string> v;
        Str::SplitTrimNoNull(p, v, ",");
        for (string* it = v.begin(); ! v.end(); it = v.next())
        {
            if ("reg" == *it)
            {
                m_match |= MATCH_REG;
                //检查是否合格
                delete m_reg;
                m_reg = new Reg(m_value);
                if (! m_reg->Init())
                {
                    delete m_reg;
                    m_reg = NULL;
                    return false;
                }
            }
            else if ("icase" == *it)
            {
                m_match |= MATCH_ICASE;
            }
            else if ("int" == *it)
            {
                m_match |= MATCH_INT;
            }
            else if ("<" == *it)
            {
                m_match |= MATCH_LT;
            }
            else if (">" == *it)
            {
                m_match |= MATCH_GT;
            }
            else if ("=" == *it)
            {
                m_match |= MATCH_EQ;
            }
            else if ("!=" == *it)
            {
                m_match |= MATCH_NE;
            }
            else if ("fmt_err" == *it)
            {
                m_match |= MATCH_FMT_ERR;
            }
            else if ("fmt_ok" == *it)
            {
                m_match |= MATCH_FMT_OK;
            }
            else if ("no_exist" == *it)
            {
                m_match |= MATCH_NO_EXIST;
            }
            else if ("exist" == *it)
            {
                m_match |= MATCH_EXIST;
            }
            else if ("shell" == *it)
            {
                m_match |= MATCH_SHELL;
                const char* pp = e->Attribute("match_shell_cmd");
                if (pp)
                {
                    m_matchShellCmd = pp;
                }
                else
                {
                    GLOG(IM_ERROR, "no match_shell_cmd for \"match=shell\" attr");
                    return false;
                }
                Str::SplitEscaped(m_matchShellCmd, m_matchShellCmdArr);
                if (m_matchShellCmdArr.size() == 0)
                {
                    GLOG(IM_ERROR, "match_shell_cmd is null for \"match=shell\" attr");
                    return false;
                }
            }
            else if ("replaceshvar")
            {
                m_match |= MATCH_SHELL_RVAR;
            }
            else
            {
                GLOG(IM_ERROR, "no support \"%s\" for match attr", it->c_str());
                return false;
            }
        }
        GLOG(IM_DEBUG, "read match = %04X", m_match);
    }
    p = e->Attribute("from");
    if (p)
    {
        EList<string> v;
        Str::SplitTrimNoNull(p, v, ",");
        for (string* it = v.begin(); ! v.end(); it = v.next())
        {
            if (0 == strcmp(it->c_str(), "file"))
            {
                const char* file = e->GetText();
                if (file && *file != '$' &&0 != access(file, R_OK))
                {
                    GLOG(IM_ERROR, "read file failed: %s", file);
                    return false;
                }
                m_from |= FROM_FILE;
            }
            /*
            else if (0 == strcmp(it->c_str(), "shell"))
            {
                m_from |= FROM_SHELL;
            }
            else if (0 == strcmp(it->c_str(), "head"))
            {
                m_from |= FROM_SHELL_HEAD;
            }
            else if (0 == strcmp(it->c_str(), "body"))
            {
                m_from |= FROM_SHELL_BODY;
            }
            else if (0 == strcmp(it->c_str(), "rvalue"))
            {
                m_from |= FROM_SHELL_RVALUE;
            }
            else if (0 == strcmp(it->c_str(), "replacevar"))
            {
                m_from |= FROM_SHELL_RVAR;
            }*/
            else if (0 == strcmp(it->c_str(), "cached"))
            {
                m_from |= FILE_CACHED;
            }
            else if (0 == strcmp(it->c_str(), "var"))
            {
                m_from |= FROM_VAR;
            }
            else if (! LoadOP(*it, e))
            {
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
            if (0 == strcmp(it->c_str(), "hide"))
            {
                m_op |= OP_HIDE;
            }
            else if (0 == strcmp(it->c_str(), "no_pack"))
            {
                m_op |= OP_NOPACK;
            }
            else if (! LoadOP(*it, e))
            {
                return false;
            }
        }
    }
    p = e->Attribute("repeated_num");
    if (p) 
    {
        m_repeatedNum = p;
    }
    return true;
}

string DatNode::DebugString(int b)
{
    string blank(b, ' ');
    string s = blank;
    s += m_name;
    if (0 == m_subs.size())
    {
        s = s + " : " + Str::ToPrint(m_value) + "\n";
        return s;
    }
    s += " {\n";
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        s += ((DatNode*)(*it))->DebugString(b + 2);
    }
    s += blank + " }\n";
    return s;
}

string DatNode::DebugXmlString(int b)
{
    string blank(b, ' ');
    string s = blank;
    s += "<" + m_name + ">";
    if (0 == m_subs.size())
    {
        s = s + Str::ToPrintXml(m_value) + "</" + m_name + ">\n";
        return s;
    }
    s += "\n";
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        s += ((DatNode*)(*it))->DebugXmlString(b + 2);
    }
    s += blank + "</" + m_name + ">\n";
    return s;
}


bool DatNode::Hide()
{
    return m_op & OP_HIDE;
}

bool DatNode::NoPack()
{
    return m_op & OP_NOPACK;
}

void DatNode::SetAttr(Node* n)
{
    Node::SetAttr(n);
    DatNode* dn = (DatNode*)n;
    m_from = dn->m_from;
    m_match = dn->m_match;
    //dn->m_reg = new Reg(*m_reg);
    m_matchShellCmd = dn->m_matchShellCmd;
    m_repeatedNum = dn->m_repeatedNum;
    string* pc;
    for (pc = dn->m_matchShellCmdArr.begin();
            ! dn->m_matchShellCmdArr.end(); 
            pc = dn->m_matchShellCmdArr.next())
    {
        m_matchShellCmdArr.push_back(*pc);
    }
    return ;
}

void DatNode::ProcessDynamicRepeatedNode(RRMessage* msg, 
        vector<Node*>* newNodes, vector<Node*>* delNodes)
{
    // 遍历子节点, 
    vector<Node*> news;
    vector<Node*> dels;
    for (Node** i = m_subs.begin(); ! m_subs.end(); i = m_subs.next())
    {
        ((DatNode*)(*i))->ProcessDynamicRepeatedNode(msg, &news, &dels);
    }
    for (vector<Node*>::iterator it = news.begin(); it != news.end(); ++it)
    {
        m_subs.push_back(*it);
    }
    for (vector<Node*>::iterator it = dels.begin(); it != dels.end(); ++it)
    {
        m_subs.erase(*it);
    }

    // 处理本节点

    if (m_repeatedNum != "" && m_parent != NULL) // 不能是根节点
    {
        int repeatedNum = atoi(m_repeatedNum.c_str());
        if (0 == repeatedNum && m_repeatedNum[0] != '0')
        {// 不是数字
            if (m_repeatedNum[0] != '$')
            {
                GLOG(IM_ERROR, "repeated_num<%s> is not int, ignore", m_repeatedNum.c_str());
            }
            // 是变量
            bool found = false;
            repeatedNum = atoi(msg->GetVar(m_repeatedNum, found));
        }
        if (0 == repeatedNum)
        {// 需要删除本节点
            delNodes->push_back(this);
        }
        for (int i = 1; i < repeatedNum; ++i)
        {
            Node* p = m_parent;
            DatNode* n = new DatNode;
            n->DeepCopy(this);
            n->m_parent = p;
            newNodes->push_back(n);
        }
    }
    return ;
}

void DatNode::DeepCopy(DatNode* n)
{
    m_name = n->m_name;
    m_value = n->m_value;
    SetAttr(n);
    for (Node** i = n->m_subs.begin(); ! n->m_subs.end(); i = n->m_subs.next()) 
    {
        DatNode* p = new DatNode;
        p->DeepCopy((DatNode*)*i);
        p->m_parent = this;
        m_subs.push_back(p);
    }
}
