#include <stdlib.h>
#include <stdint.h>
#include "BaseMock.h"
#include "INIConf.h"
#include "Log.h"
#include "ProcTitle.h"
#include "File.h"
#include "ProcInfo.h"
#include "Time.h"
#include "Str.h"

using namespace std;

BaseMock::BaseMock(const string& name) : m_name(name), 
    m_mmapProcInfo(NULL), m_realInfoTid(0)
{
}

BaseMock::~BaseMock()
{
    if (m_realInfoTid != 0)
    {
        pthread_cancel(m_realInfoTid);
        pthread_join(m_realInfoTid, NULL);
    }
}

void BaseMock::SetName(const string& name)
{
    m_name = name;
}

string& BaseMock::GetName()
{
    return m_name;
}

int BaseMock::GetWorkers()
{
    return m_workers;
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
        { m_name.c_str(), "workers", ISet::Int,  (uint64_t)&m_workers     , "1"  },
        { m_name.c_str(), "format",  ISet::Str,  (uint64_t)&m_fmtFile     , NULL },
        { m_name.c_str(), "data",    ISet::Str,  (uint64_t)&m_datFile     , NULL },
        { m_name.c_str(), "message", SetMsgFile, (uint64_t)this           , "off"},
        { m_name.c_str(), "mode",    SetMode,    (uint64_t)this           , "RS" },
        { m_name.c_str(), "realinfo",ISet::Str,  (uint64_t)&m_realInfoFile, ""   },
        { NULL,   NULL,      NULL,       0,      NULL  }
    };
    if (! g_iniConf->Get(c, 0))
    {
        return false;
    }
    return true;
}

void* BaseMock::OutPutRealInfo(void* bm)
{
    BaseMock* baseMock = (BaseMock*) bm;
    FILE* realInfoFileHandle = NULL;
    bool needClose = false;
    if (baseMock->m_realInfoFile == "/dev/stdout")
    {
        realInfoFileHandle = stdout;
    }
    else if (baseMock->m_realInfoFile == "/dev/stderr")
    {
        realInfoFileHandle = stderr;
    }
    else 
    { 
        realInfoFileHandle = fopen(baseMock->m_realInfoFile.c_str(), "a+");
        needClose = true;
    }
    int rotate = 0;
    ProcInfo old;
    while (true)
    {
        ProcInfo now;
        for (int i = 0; i < baseMock->m_workers; ++i)
        {
            now += *(baseMock->m_mmapProcInfo + i);
        }
        if (rotate % 20 == 0)
        {// 打印头部
            fprintf(realInfoFileHandle, "Time                ---bytin-- --bytout-- ---qpsin-- --qpsout-- ----rt----\n");
        }
        if (rotate != 0)
        {// 非第一次
            ProcInfo tmp = now - old;
            fprintf(realInfoFileHandle, "%s %10s %10s %10s %10s %10s\n", 
                    Time::StrTimeNow().c_str(), 
                    Str::ToKMGT(tmp.GetRequestNumInBytes()).c_str(),
                    Str::ToKMGT(tmp.GetRequestNumOutBytes()).c_str(),
                    Str::ToKMGT(tmp.GetRequestNumIn()).c_str(),
                    Str::ToKMGT(tmp.GetRequestNumOut()).c_str(),
                    tmp.GetRequestNumOut() == 0 ? "0.00" : Str::ToKMGT(tmp.GetRequestTimeMS()/tmp.GetRequestNumOut()).c_str()
                   );
        }
        fflush(realInfoFileHandle);
        old = now;
        rotate++;
        sleep(1);
    }
    if (needClose)
    {
        fclose(realInfoFileHandle);
    }
}

void BaseMock::Run(ProcInfo* procInfo)
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
                m_mmapProcInfo = procInfo + i;
                goto WORKER;
            default:
                m_wpids.insert(pid);
                break;
        }
    }
    m_mmapProcInfo = procInfo;// master 指向头部
    if (! m_realInfoFile.empty()) 
    {
        int ret = pthread_create(&m_realInfoTid, NULL, BaseMock::OutPutRealInfo, this);
        if (ret != 0) 
        {
            GLOG(IM_ERROR, "create thread error, errno=%d", errno);
            return ;
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
