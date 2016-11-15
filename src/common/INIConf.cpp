#include <fstream>
#include <stdlib.h>
#include <stdint.h>
#include "INIConf.h"
#include "Log.h"
#include "Str.h"
#include "System.h"

using namespace std;

INIConf* g_iniConf = NULL;

bool ISet::Int(const string& s, void* d, uint64_t o)
{   
    if (s.empty())
    {
        return false;
    }
    char* p = (char*)d;
    int* np = (int*)(p + o);
    *np = atoi(s.c_str());
    return true;
}

bool ISet::Str(const string& s, void* d, uint64_t o)
{
    char* p = (char*)d;
    string* np = (string*)(p + o);
    *np = s;
    return true;
}

bool ISet::Flag(const string& s, void* d, uint64_t o)
{
    if (s.empty())
    {
        return false;
    }
    char* p = (char*)d;
    bool* np = (bool*)(p + o);
    if ("on" == s)
    {
        *np = true;
    }
    else if ("off" == s)
    {
        *np = false;
    }
    else
    {
        return false;
    }
    return true;
}

bool ISet::Size(const string& s, void* d, uint64_t o)
{
    if (s.empty())
    {
        return false;
    }
    char unit = s[s.length()-1];
    int scale = 1;
    switch (unit)
    {
        case 'K':
        case 'k':
            scale = 1024;
            break;
        case 'M':
        case 'm':
            scale = 1024 * 1024;
            break;
        case 'G':
        case 'g':
            scale = 1024 * 1024 * 1024;
            break;
        default:
            scale = 1;
    }
    int size = atoi(s.c_str());
    char* p = (char*)d;
    int* np = (int*)(p + o);
    *np = size * scale;
    return true;
}

bool INIConf::Load(const string& ini)
{
    static set<string> inis;
    if (inis.find(ini) != inis.end())
    {
        GLOG(IM_DEBUG, "load %s repeated. ignore", ini.c_str());
        return true;
    }
    inis.insert(ini);
    ifstream f(ini.c_str());
    if (!f)
    {
        GLOG(IM_ERROR, "can't open %s", ini.c_str());
        return false;
    }

    string line;
    string sec = "";
    int lineNum = 0;
    while (f.good() && getline(f, line))
    {
        ++lineNum;
        Str::Trim(line);//去除首尾空格等
        if (line.empty() || '#' == line[0])
        {//为空或者是注释
            continue;
        }
        if (0 == strncmp("include ", line.c_str(), sizeof("include ") - 1))
        {
            string newFile = line.substr(sizeof("include ") - 1);
            if (! newFile.empty())
            {
                string::size_type p = newFile.find('*');
                if (string::npos != p)
                {
                    System cmd;
                    cmd.Add("ls");
                    cmd.Add(newFile.c_str());
                    stringstream ss(cmd.Cmd());
                    string tmp;
                    while (ss >> tmp)
                    {
                        if (! Load(tmp))
                        {
                            GLOG(IM_ERROR, "load file %s error in %s:%d", 
                                    tmp.c_str(), ini.c_str(), lineNum);
                            return false;
                        }
                    }
                }
                else
                {
                    GLOG(IM_DEBUG, "load %s",   newFile.c_str());
                    if (! Load(newFile))
                    {
                        GLOG(IM_ERROR, "load file %s error in %s:%d", 
                                newFile.c_str(), ini.c_str(), lineNum);
                        return false;
                    }
                }
            }
            continue;
        }
        if ('[' == line[0])
        {//是一个section
            //找到下一个]
            string::size_type p = line.find(']');
            if (string::npos == p)
            {
                GLOG(IM_ERROR, "not found ] in %s:%d", ini.c_str(), lineNum);
                return false;
            }
            sec = line.substr(1, p - 1);
            Str::Trim(sec);
            continue;
        }
        else
        {//是一个key = val
            string::size_type p = line.find('=');
            if (string::npos == p)
            {
                GLOG(IM_ERROR, "not found = in %s:%d", ini.c_str(), lineNum);
                return false;
            }
            string k, v;
            k = line.substr(0, p);
            v = line.substr(p + 1);
            Str::Trim(k);
            Str::Trim(v);
            if (k.empty())
            {
                GLOG(IM_ERROR, "key is null in %s:%d", ini.c_str(), lineNum);
                return false;
            }
            if (v.empty())
            {
                GLOG(IM_ERROR, "value is null in %s:%d", ini.c_str(), lineNum);
                return false;
            }
            KVMap& kvmap = m_secMap[sec];
            if (kvmap.find(k) != kvmap.end())
            {
                GLOG(IM_ERROR, "%s is duplicate in %s:%d", 
                    k.c_str(), ini.c_str(), lineNum);
                return false;
            }
            m_secMap[sec][k] = v;
        }
    }
    GLOG(IM_DEBUG, "Load ini conf:%s ok!", ini.c_str());
    return true;
}

string INIConf::Dump() const
{
    string s = "";
    SecMap::const_iterator it = m_secMap.begin();
    while (it != m_secMap.end())
    {
        s += "[" + it->first + "]\n";
        const KVMap& kv = it->second;
        KVMap::const_iterator itKV = kv.begin();
        while (itKV != kv.end())
        {
            s += itKV->first + "=" + itKV->second + "\n";
            ++itKV;
        }
        ++it;
    }
    return s;
}

bool INIConf::Get(const string& sec, const string& key, string& val)
{
    SecMap::const_iterator it = m_secMap.find(sec);
    if (m_secMap.end() == it)
    {
        return false;
    }
    KVMap::const_iterator kvit = it->second.find(key);
    if (it->second.end() == kvit)
    {
        return false;
    }
    val = kvit->second;
    return true;
}

bool INIConf::Get(const string& sec, 
        const string& key, Set set, void* data, const char* def)
{
    string val;
    if (! Get(sec, key, val))
    {
        if (! def)
        {// 没有默认值
            return false;
        }
        val = def;
    }
    return set(val, data, 0);
}

bool INIConf::Get(const Command* c, void* data)
{
    const Command* p = c;
    while (NULL != p && NULL != p->sec)
    {
        string val;
        if (! Get(p->sec, p->key, val))
        {
            if (! p->def)
            {// 没有默认值
                GLOG(IM_ERROR, "can't find cmd<%s:%s>\n", p->sec, p->key);
                return false;
            }
            val = p->def;
        }
        if (! p->set(val, data, p->offset))
        {
            GLOG(IM_ERROR, "set cmd failed<%s:%s>\n", p->sec, p->key);
            return false;
        }
        ++p;
    }
    return true;
}
