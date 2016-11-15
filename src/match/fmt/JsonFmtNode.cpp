#include "JsonFmtNode.h"
#include "DatNode.h"
#include "Log.h"
#include "Str.h"
#include <list>

using namespace std;

JsonFmtNode::JsonValueType::JsonValueType(IMOCK::Json::Value* _v, int _t)
{
    v = _v;
    type = _t;
}

bool JsonFmtNode::ParseSubNode(RRMessage* msg, _DNS& dns, EList<_DNS>* add)
{
    if (dns.s.empty())
    {
        return true;
    }
    IMOCK::Json::Reader reader;
    IMOCK::Json::Value root;
    if (! reader.parse(dns.s, root))
    {
        GLOG(IM_ERROR, "parse json error:%s\n%s", reader.getFormattedErrorMessages().c_str(), dns.s.c_str());
        return false;
    }
    if (root.type() == IMOCK::Json::arrayValue)
    {// 附加到add里
        if (NULL == add)
        {
            GLOG(IM_ERROR, "oh, no, fuck! tell hqin6@163.com please");
            return false;
        }
        int size = root.size();
        for (int i = 0; i < size; ++i)
        {
            _DNS tmp;
            _DNS *ptmp;
            bool isAdd = true;
            if (i == 0) 
            {// 第一个使用dns
                isAdd = false;
                ptmp = &dns;
            }
            if (isAdd)
            {
                ptmp = &tmp;
                tmp.dn = new DatNode();
                tmp.dn->SetName(m_name);
            }

            IMOCK::Json::Value::Members m = root[i].getMemberNames();
            for (IMOCK::Json::Value::Members::iterator it = m.begin();
                    it != m.end(); ++it)
            {
                if (! ParseSubNodeJson(msg, *ptmp, root[i][*it], GetSub(*it), *it))
                {
                    if (isAdd)
                    {
                        delete tmp.dn;
                    }
                    return false;
                }
            }
            if (isAdd)
            {
                add->push_back(*ptmp);
            }
        }
    }
    else if (root.type() == IMOCK::Json::objectValue)
    {
        IMOCK::Json::Value::Members m = root.getMemberNames();
        for (IMOCK::Json::Value::Members::iterator it = m.begin();
                it != m.end(); ++it)
        {
            if (! ParseSubNodeJson(msg, dns, root[*it], GetSub(*it), *it))
            {
                return false;
            }
        }
    }
    else
    {
        GLOG(IM_ERROR, "json just support object root node");
        return false;
    }
    return true;
}

bool JsonFmtNode::ParseSubNodeJson(RRMessage* msg, 
        _DNS& dns, IMOCK::Json::Value& v, Node* node, const string& name)
{
    IMOCK::Json::Value::Members m;
    int size;
    DatNode* dn;
    switch (v.type())
    {
        case IMOCK::Json::arrayValue:
            size = v.size();
            for (int i = 0; i < size; ++i)
            {
                if (! ParseSubNodeJson(msg, dns, v[i], node, name))
                {
                    return false;
                }
            }
            break;
        case IMOCK::Json::objectValue:
            m = v.getMemberNames();
            dn = new DatNode;
            dn->SetName(name);
            dns.dn->AddChild(dn);
            for (IMOCK::Json::Value::Members::iterator it = m.begin();
                    it != m.end(); ++it)
            {
                _DNS tmp;
                tmp.dn = dn;
                if (! ParseSubNodeJson(msg, tmp, v[*it], node ? node->GetSub(*it): NULL, *it))
                {
                    return false;
                }
            }
            break;
        default:
            string vv = v.asString();
            if (node)
            {
                EList<string> vec;
                int idx = 0;
                bool useIdx = false;
                if (! ((FmtNode*)node)->GetNodeStr(msg, vv, idx, vec, useIdx))
                {
                    return false;
                }
                EList<_DNS> vs;
                if (!((FmtNode*)node)->ParseDG(msg, vs, &vec))
                {
                    return false;
                }
                for (_DNS* i = vs.begin(); ! vs.end(); i = vs.next())
                {
                    //增加子节点
                    dns.dn->AddChild(i->dn);
                }
            }
            else
            {
                DatNode* dn = new DatNode();
                dn->SetName(name);
                dn->SetValue(vv);
                dns.dn->AddChild(dn);
                GLOG(IM_DEBUG, "add key<%s> = val<%s>", name.c_str(), vv.c_str());
            }
            break;
    }
    return true;
}

bool JsonFmtNode::SerializeOne(RRMessage* dst, 
        DatNode* dn, string& s, RRMessage* src)
{
    if (dn->NoPack())
    {
        DirectFill(src, dn, s);
        return true;
    }
    IMOCK::Json::Value root;
    if (! SerializeSub(dst, dn, src, root, this))
    {
        return false;
    }
    IMOCK::Json::FastWriter fastwriter;
    s = fastwriter.write(root);
    s.erase(s.length()-1);
    dn->ExecOP(s, src, s);
    return true;
}

void JsonFmtNode::FmtJsonNode(IMOCK::Json::Value& root, string& v, int type, long op)
{
    if (type & NVT_INT)
    {
        int i = atoi(v.c_str());
        char buf[16];
        memcpy(buf, &i, sizeof(int));
        root = string(buf, sizeof(int));
    }
    else if (op & OP_NO_AUTO_CVT)
    {
        root = v;
    }
    else 
    {
        int i = 0;
        double d = 0;
        if (v == "null")
        {
            root = IMOCK::Json::Value(IMOCK::Json::nullValue);
        }
        else if (v == "true")
        {
            root = IMOCK::Json::Value(true);
        }
        else if (v == "false")
        {
            root = IMOCK::Json::Value(false);
        }
        else if (Str::AtoI(v, i))
        {
            root = IMOCK::Json::Value(i);
        }
        else if (Str::AtoF(v, d))
        {
            root = IMOCK::Json::Value(d);
        }
        else
        {
            root = v;
        }
    }
}

bool JsonFmtNode::SerializeSub(RRMessage* dst,
        DatNode* dn, RRMessage* src, IMOCK::Json::Value& root, FmtNode* fn)
{
    EList<Node*>& vec = dn->GetSubs();

    long op = 0;// 对本节点的处理
    if (fn) 
    {
        op |= fn->GetOP();
    }
    if (dn)
    {
        op |= dn->GetOP();
    }

    if (fn && (fn->GetType() & NVT_JLEAF))
    {// 强制申明叶子节点,里面不再展开
        EList<DatNode*> tmp;
        tmp.push_back(dn);
        string r = "";
        if (false == fn->Serialize(dst, tmp, r, src))
        {
            return false;
        }
        FmtJsonNode(root, r, fn->GetType(), op);
    }
    else if (0 == vec.size())
    {// 非强制声明的叶子节点,里面不再展开
        string tmp = dn->GetValue(src);
        if (fn)
        {
            fn->TokOP(src, tmp);
        }
        FmtJsonNode(root, tmp, fn == NULL ? NVT_NONE : fn->GetType(), op);
    }
    else
    {// 展开子节点
        map<string, list<JsonValueType> > mp;
        for (Node** j = vec.begin(); ! vec.end(); j = vec.next())
        {
            DatNode* d = (DatNode*)*j;
            if (d->Hide())
            {
                continue;
            }
            IMOCK::Json::Value* tmp = new IMOCK::Json::Value;
            FmtNode* f = (FmtNode*)(fn ? fn->GetSub(d->GetName()) : NULL);
            JsonFmtNode::SerializeSub(dst, d, src, *tmp, f);
            mp[d->GetName()].push_back(JsonValueType(tmp, f ? f->GetType(): NVT_NONE));
        }
        for (map<string, list<JsonValueType> >::iterator it = mp.begin();
                it != mp.end(); ++it)
        {
            if (1 == it->second.size() && ! (it->second.begin()->type & NVT_JARR))
            {
                IMOCK::Json::Value* tmp = (it->second.begin()->v);
                root[it->first] = *tmp;
                delete tmp;
            }// 余下肯定是数组
            else
            {//数组
                for (list<JsonValueType>::iterator i = it->second.begin(); 
                        i != it->second.end(); i++)
                {
                    IMOCK::Json::Value* tmp = (i->v);
                    if (tmp->type() == IMOCK::Json::nullValue && it->second.size() == 1)
                    {
                        root[it->first].resize(0);
                    }
                    else
                    {
                        root[it->first].append(*tmp);
                    }
                    delete tmp;
                }
            }
        }
    }
    return true;
}

bool JsonFmtNode::Serialize(RRMessage* dst, 
        EList<DatNode*>& dn, 
        std::string& s, RRMessage* src)
{
    if (m_repeatedSplit && (m_type & NVT_JARR))
    {
        GLOG(IM_ERROR, "error: node must not be repeated and jarr");
        return false;
    }
    if (m_repeatedSplit)
    {// 如果是普通重复属性,直接用父类方法
        return FmtNode::Serialize(dst, dn, s, src);
    }

    if (dn.size() > 1 || (m_type & NVT_JARR))
    {// 如果是数组
        IMOCK::Json::Value rr;
        for (DatNode** it = dn.begin(); ! dn.end(); it = dn.next())
        {// hide属性无效 
            if ((*it)->Hide())
            {
                continue;
            }
            if ((*it)->NoPack())
            {
                DirectFill(src, *it, s);
                continue;
            }
            IMOCK::Json::Value root;
            if (! SerializeSub(dst, *it, src, root, this))
            {
                return false;
            }
            rr.append(root);
        }
        IMOCK::Json::FastWriter fastwriter;
        string tmp = fastwriter.write(rr);
        tmp.erase(tmp.length()-1);
        s += tmp;
        return true;
    }
    else
    {// 普通,直接使用父类方法
        return FmtNode::Serialize(dst, dn, s, src);
    }
}
