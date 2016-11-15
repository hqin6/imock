#include <stdlib.h>
#include <stdint.h>
#include "BaseMock.h"
#include "INIConf.h"
#include "Log.h"
#include "ProcTitle.h"
#include "File.h"

using namespace std;

BaseMock::BaseMock(const string& name) : m_name(name)
{
}

BaseMock::~BaseMock()
{
}

void BaseMock::SetName(const string& name)
{
    m_name = name;
}

bool BaseMock::Init()
{
    if (! g_iniConf)
    {
        GLOG(IM_ERROR, "no INI configure");
        return false;
    }
    //获取配置
    Command c[] = {
        { m_name.c_str(), "workers", ISet::Int,  (uint64_t)&m_workers, "1"  },
        { m_name.c_str(), "format",  ISet::Str,  (uint64_t)&m_fmtFile, NULL },
        { m_name.c_str(), "data",    ISet::Str,  (uint64_t)&m_datFile, NULL },
        { m_name.c_str(), "message", SetMsgFile, (uint64_t)this      , "off"},
        { m_name.c_str(), "mode",    SetMode,    (uint64_t)this      , "RS" },
        { NULL,   NULL,      NULL,       0,      NULL  }
    };
    if (! g_iniConf->Get(c, 0))
    {
        return false;
    }
    return true;
}

void BaseMock::Run()
{
    if (0 != BeforeFork())
    {
        return ;
    }
    //循环fork不同的mock
    for (int i = 0; i < m_workers; ++i)
    {
        int pid = fork();
        switch (pid)
        {
            case -1:
                GLOG(IM_ERROR, "fork error, errno=%d", errno);
                return ;
            case 0:
                goto WORKER;
            default:
                m_wpids.insert(pid);
                break;
        }
    }
    return ;

    //mock进程
WORKER:
    g_iniConf->Get(m_name.c_str(), "log", LoadLog, NULL);
    GLOG(IM_INFO, "worker begin...");
    //设置进程名
    string procName = "imock: worker " + RoleName() + " " + m_name;
    ProcTitle::Set(procName);
    //调用afterfork
    if (0 != AfterFork())
    {
        exit(-1);
    }
    //执行Loop
    int ret = Loop();
    //退出
    GLOG(IM_INFO, "worker exited:%d", ret);
    exit(ret);
}

void BaseMock::WriteMsg(const std::string& msg)
{
    if (! m_msgFile.empty() && m_msgFile != "off")
    {
        File::Write(m_msgFile, msg, m_msgFileMode);
    }
    return ;
}

int BaseMock::BeforeFork()
{
    return 0;
}

int BaseMock::AfterFork()
{
    return 0;
}

bool BaseMock::SetMode(const string& s, void* d, uint64_t o)
{
    char* p = (char*)d;
    BaseMock* obj = (BaseMock*)(p + o);
    obj->m_mode = 0;
    for (int i = 0; i < (int)s.size(); ++i)
    {
        if ('R' == s[i])
        {
            obj->m_mode |= MODE_RECV;
        }
        else if ('S' == s[i])
        {
            obj->m_mode |= MODE_SEND;
        }
        else
        {
            return false;
        }
    }
    if (0 == obj->m_mode)
    {
        return false;
    }
    return true;
}

bool BaseMock::SetMsgFile(const string& s, void* d, uint64_t o)
{
    char* p = (char*)d;
    BaseMock* obj = (BaseMock*)(p + o);
    stringstream ss(s);
    ss >> obj->m_msgFile >> obj->m_msgFileMode;
    if (obj->m_msgFile.empty())
    {
        return false;
    }
    if (obj->m_msgFileMode.empty())
    {
        obj->m_msgFileMode = "w";
    }
    else
    {
        if (obj->m_msgFileMode != "w" && obj->m_msgFileMode != "w+" && 
                obj->m_msgFileMode != "a" && obj->m_msgFileMode != "a+")
        {
            return false;
        }
    }
    return true;
}
