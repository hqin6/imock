#include <sstream>
#include <sys/time.h>
#include "Node.h"
#include "Log.h"
#include "Str.h"
#include "Zlib.h"
#include "Var.h"
#include "File.h"
#include "System.h"

using namespace std;

OPHandler::OPHandler(_s_cs h)
{
    type = S_CS;
    handler.s_cs = h;
}
OPHandler::OPHandler(_v_s_cs h)
{
    type = V_S_CS;
    handler.v_s_cs = h;
}
OPHandler::OPHandler(_s_s_r h)
{
    type = S_S_R;
    handler.s_s_r = h;
}
OPHandler::OPHandler(_s_s_r_n_cs h)
{
    type = S_S_R_N_CS;
    handler.s_s_r_n_cs = h;
}
void OPHandler::Exec(string& v, RRMessage* m, Node* n, const string& rv)
{
    switch (type)
    {
        case S_CS:
            v = handler.s_cs(v);
            break;
        case V_S_CS:
            handler.v_s_cs(v, "");
            break;
        case S_S_R:
            v = handler.s_s_r(v, m);
            break;
        case S_S_R_N_CS:
            v = handler.s_s_r_n_cs(v, m, n, rv);
            break;
    }
    return ;
}

map<string, string> Node::s_shellCache;

Node::Node() : m_parent(NULL), m_op(0)
{
}

Node::~Node()
{
    Clear();
}

void Node::Clear()
{
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        delete *it;
    }
    m_subs.clear();
}

bool Node::Load(TiXmlElement* e, Node* parent)
{
    if (! s_nf)
    {
        GLOG(IM_ERROR, "no node factory.");
        return false;
    }
    const char* name = e->Value();
    if (! name)
    {
        GLOG(IM_ERROR, "e->Value() error");
        return false;
    }
    m_name = name;
    m_parent = parent;
    if (m_parent)
    {
        m_parent->m_subs.push_back(this);
    }
    if (! LoadAttr(e))
    {
        return false;
    }
    for (TiXmlElement* c = e->FirstChildElement();
            c; c = c->NextSiblingElement())
    {
        Node* p = s_nf(c);
        if (!p || ! p->Load(c, this))
        {
            return false;
        }
    }
    return true;
}

bool Node::LoadOP(const string& op, TiXmlElement* e)
{
    if (op == "b64enc")
    {
        m_op |= OP_B64ENC;
        m_opFun.push_back(Str::B64Enc);
    }
    else if (op == "b64enc_safe")
    {
        m_op |= OP_B64ENC_SAFE;
        m_opFun.push_back(Str::SafeB64Enc);
    }
    else if (op == "b64enc_noappend")
    {
        m_op |= OP_B64ENC_NOAPPEND;
        m_opFun.push_back(Str::B64EncNoAppend);
    }
    else if (op == "b64enc_safe_noappend")
    {
        m_op |= OP_B64ENC_SAFE_NOAPPEND;
        m_opFun.push_back(Str::SafeB64EncNoAppend);
    }
    else if (op == "b64dec")
    {
        m_op |= OP_B64DEC;
        m_opFun.push_back(Str::B64Dec);
    }
    else if (op == "b64dec_safe")
    {
        m_op |= OP_B64DEC_SAFE;
        m_opFun.push_back(Str::SafeB64Dec);
    }
    else if (op == "b64dec_noappend")
    {
        m_op |= OP_B64DEC_NOAPPEND;
        m_opFun.push_back(Str::B64DecNoAppend);
    }
    else if (op == "b64dec_safe_noappend")
    {
        m_op |= OP_B64DEC_SAFE_NOAPPEND;
        m_opFun.push_back(Str::SafeB64DecNoAppend);
    }
    else if (op == "urlenc")
    {
        m_op |= OP_URL_ENC;
        m_opFun.push_back(Str::UrlEnc);
    }
    else if (op == "urldec")
    {
        m_op |= OP_URL_DEC;
        m_opFun.push_back(Str::UrlDec);
    }
    else if (op == "gzip")
    {
        m_op |= OP_GZIP;
        m_opFun.push_back(Zlib::GZip);
    }
    else if (op == "gunzip")
    {
        m_op |= OP_GUNZIP;
        m_opFun.push_back(Zlib::GUnZip);
    }
    else if (op == "zip")
    {
        m_op |= OP_ZIP;
        m_opFun.push_back(Zlib::Zip);
    }
    else if (op == "unzip")
    {
        m_op |= OP_UNZIP;
        m_opFun.push_back(Zlib::UnZip);
    }
    else if (op == "replacevar")
    {
        m_op |= OP_RVAR;
        m_opFun.push_back(Node::ReplaceVar);
    }
    else if (op == "rtrim")
    {
        m_op |= OP_RTRIM;
        m_opFun.push_back(Str::RTrim);
    }
    else if (op == "ltrim")
    {
        m_op |= OP_LTRIM;
        m_opFun.push_back(Str::LTrim);
    }
    else if (op == "trim")
    {
        m_op |= OP_TRIM;
        m_opFun.push_back(Str::Trim);
    }
    else if (op == "shell")
    {
        m_op |= OP_SHELL;
        m_opFun.push_back(Node::Shell);
        const char* p = e->Attribute("shell_cmd");
        if (p)
        {
            m_shellCmd = p;
        }
        else
        {
            p = e->GetText();
            if (!p || *p == '\0')
            {
                GLOG(IM_ERROR, "no shell_cmd for \"op=shell\" attr");
                return false;
            }
            GLOG(IM_WARN, "please use shell_cmd for \"op=shell\" attr");
            m_shellCmd = p;
        }
        Str::SplitEscaped(m_shellCmd, m_shellCmdArr);
        if (m_shellCmdArr.size() == 0)
        {
            GLOG(IM_ERROR, "shell_cmd is null for \"op=shell\" attr");
            return false;
        }
    }
    else if (op == "head")
    {
        m_op |= OP_SHELL_HEAD;
    }
    else if (op == "body")
    {
        m_op |= OP_SHELL_BODY;
    }
    else if (op == "rvalue")
    {
        m_op |= OP_SHELL_RVALUE;
    }
    else if (op == "replaceshvar")
    {
        m_op |= OP_SHELL_RVAR;
    }
    else if (op == "cached")
    {
        m_op |= OP_CACHED;
    }
    else if (op == "esc")
    {
        m_op |= OP_ESC;
        m_opFun.push_back(Str::Esc);
    }
    else if (op == "nofile")
    {
        m_op |= OP_SHELL_NO_IN_FILE;
    }
    else if (op == "nocvt")
    {
        m_op |= OP_NO_AUTO_CVT;
    }
    else
    {
        GLOG(IM_ERROR, "no support \"%s\" for \"op\"", op.c_str());
        return false;
    }
    return true;
}

void Node::SetFactory(NodeFactory nf)
{
    if (NULL == nf)
    {
        s_nf = NULL;
        pthread_mutex_unlock(s_factoryMutex);
    }
    else
    {
        pthread_mutex_lock(s_factoryMutex);
        s_nf = nf;
    }
}

Node::NodeFactory Node::s_nf = NULL;

pthread_mutex_t* Node::s_factoryMutex = Node::NewFactoryMutex();

pthread_mutex_t* Node::NewFactoryMutex()
{
    pthread_mutex_t* p = new pthread_mutex_t;
    pthread_mutex_init(p, NULL);
    return p;
}

bool Node::LoadAttr(TiXmlElement* e)
{
    return true;
}

/*
string Node::DebugString(int b)
{
    string s = "";
    s += string(2*b, ' ') + m_name;
    string attr = AttrDebugString();
    if (! attr.empty())
    {
        s += " [" + attr + "]";
    }
    if (! m_subs.empty())
    {
        s = s + " {" + "\n";
        for (vector<Node*>::iterator it = m_subs.begin();
                it != m_subs.end(); ++it)
        {
            s += (*it)->DebugString(b + 1);
        }
        s += string(2*b, ' ') + "}";
    }
    s += "\n";
    return s;
}

string Node::AttrDebugString()
{
    return "";
}
*/

/*
bool Node::Match(const Node* n)
{
    return 0 == strcmp(n->m_name, m_name);
}

string Node::GetValue()
{
    return "";
}
*/

void Node::AddChild(Node* c)
{
    m_subs.push_back(c);
    c->m_parent = this;
}

void Node::SetName(const string& name)
{
    m_name = name;
}

const string& Node::GetName()
{
    return m_name;
}

const string Node::GetFullName()
{
    string s = "";
    if (m_parent) 
    {
        s = m_parent->GetFullName() + "." + m_name;
    }
    else
    {
        s = m_name;
    }
    return s;
}

EList<Node*>& Node::GetSubs()
{
    return m_subs;
}

long Node::GetOP()
{
    return m_op;
}

Node* Node::GetSub(const string& name)
{
    for (Node** it = m_subs.begin(); ! m_subs.end(); it = m_subs.next())
    {
        if (name == (*it)->GetName())
        {
            return *it;
        }
    }
    return NULL;
}

string Node::ReplaceVar(string& v, RRMessage* msg)
{
    if (msg)
    {
        Var m(v);
        string varName, o;
        varName = m.Next(o);
        while (varName != "")
        {
            bool found = false;
            string varVal = msg->GetVar(varName, found);
            m.Replace(0, o.size(), found ? &varVal : NULL);
            if (found)
            {
                GLOG(IM_DEBUG, "replace var:%s to %s",
                        varName.c_str(), varVal.c_str());
            }
            varName = m.Next(o);
        }
        m.ReplaceEnd();
        v = m.GetResult();
    }
    return v;
}

string Node::ExecOP(string& v, RRMessage* msg, const string& rv)
{
    for (vector<OPHandler>::iterator it = m_opFun.begin();
            it != m_opFun.end(); ++it)
    {
        it->Exec(v, msg, this, rv);
    }
    return v;
}

void Node::ShellArgs(
        EList<string>& cmdArr, 
        bool replaceVar,
        RRMessage* r, 
        vector<string>& res)
{
    string* pc;
    for (pc = cmdArr.begin();
            ! cmdArr.end(); 
            pc = cmdArr.next())
    {
        string cm = *pc;
        if (replaceVar)
        {// 替换变量
            Var m(cm);
            string varName, o;
            varName = m.Next(o);
            while (varName != "")
            {
                bool found = false;
                string varVal = r->GetVar(varName, found);
                m.Replace(0, o.size(), found ? &varVal : NULL);
                if (found)
                {
                    GLOG(IM_DEBUG, "replace var in cmd:%s to %s", 
                            varName.c_str(), varVal.c_str());
                }
                varName = m.Next(o);
            }
            m.ReplaceEnd();
            cm = m.GetResult();
        }
        res.push_back(cm);
    }
}

string Node::Shell(string& v, RRMessage* msg, Node* n, const string& rv)
{
    bool cached = n->m_op & OP_CACHED;
    if (cached)
    {//可以缓存
        map<string, string>::iterator it;
        it = s_shellCache.find(n->m_shellCmd);
        if (it != s_shellCache.end())
        {
            v = it->second;
            return v;
        }
    }
    // 传递value body head
    string  head = "", body = "", rvalue = "";
    string prefix = "/tmp/.imock.f.";
    string suffix = "";
    if (! (n->m_op & OP_SHELL_NO_IN_FILE))
    {// 构造文件后缀
        struct timeval rtime;
        gettimeofday(&rtime, NULL);
        char buf[64];
        snprintf(buf, sizeof(buf), ".%d.%d", (int)getpid(), (int)rtime.tv_usec);
        suffix = buf;
    }
    // 替换变量
    vector<string> shellArgs;
    ShellArgs(n->m_shellCmdArr, n->m_op & OP_SHELL_RVAR, msg, shellArgs);
    System cmd;
    for (vector<string>::iterator it = shellArgs.begin();
            it != shellArgs.end(); ++it)
    {
        cmd.Add(it->c_str());
    }
    // 插入rvalue/head/body
    if (n->m_op & OP_SHELL_RVALUE)
    {
        if (n->m_op & OP_SHELL_NO_IN_FILE)
        {
            cmd.Add(rv.c_str());
        }
        else
        {
            rvalue = prefix + "rvalue" + suffix;
            File::Write(rvalue, rv);
            cmd.Add(rvalue.c_str());
        }
    }
    if (n->m_op & OP_SHELL_HEAD)
    {
        if (n->m_op & OP_SHELL_NO_IN_FILE)
        {
            cmd.Add(msg->GetHeader().c_str());
        }
        else
        {
            head = prefix + "head" + suffix;
            File::Write(head, msg->GetHeader());
            cmd.Add(head.c_str());
        }
    }
    if (n->m_op & OP_SHELL_BODY)
    {
        if (n->m_op & OP_SHELL_NO_IN_FILE)
        {
            cmd.Add(msg->GetBody().c_str());
        }
        else
        {
            body = prefix + "body" + suffix;
            File::Write(body, msg->GetBody());
            cmd.Add(body.c_str());
        }
    }

    v = cmd.Cmd();
    if (cached)
    {
        s_shellCache[n->m_shellCmd] = v;
    }
    unlink(head.c_str());
    unlink(body.c_str());
    unlink(rvalue.c_str());
    return v;
}

void Node::SetAttr(Node* n)
{
    if (n)
    {
        m_opFun = n->m_opFun;
        m_op = n->m_op;
        m_shellCmd = n->m_shellCmd;
        string* pc;
        for (pc = n->m_shellCmdArr.begin();
                ! n->m_shellCmdArr.end(); 
                pc = n->m_shellCmdArr.next())
        {
            m_shellCmdArr.push_back(*pc);
        }
    }
    return ;
}
