#include "BaseClientMock.h"
#include <string>
#include <signal.h>
#include "Log.h"
#include "INIConf.h"
using namespace std;

bool BaseClientMock::s_stopFlag = false;

BaseClientMock::BaseClientMock(const string& name) : 
    BaseMock(name),
    m_fmt(NULL),
    m_dat(NULL)

{
}

BaseClientMock::~BaseClientMock()
{
    if (m_fmt)
    {
        delete m_fmt;
        m_fmt = NULL;
    }
    if (m_dat)
    {
        delete m_dat;
        m_dat = NULL;
    }
}

bool BaseClientMock::Init()
{
    if (! BaseMock::Init())
    {
        return false;
    }
    Command c[] = {
        { m_name.c_str(), "timeout", ISet::Int, (uint64_t)&m_timeout, "5000" },
        { NULL,           NULL,      NULL,      0          , NULL }
    };
    if (! g_iniConf->Get(c, 0))
    {   
        return false;
    }
    m_fmt = new Fmt();
    if (! m_fmt->Load(m_fmtFile.c_str()))
    {
        return false;
    }
    m_dat = new Dat();
    if (! m_dat->Load(m_datFile.c_str(), m_fmt, "", ""))
    {
        return false;
    }
    return true;
}

int BaseClientMock::AfterFork()
{
    if (0 != BaseMock::AfterFork())
    {
        return -1;
    }
    struct sigaction sa;
    bzero(&sa, sizeof(struct sigaction));
    //增加退出机制
    sa.sa_handler = BaseClientMock::SetStopFlag;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGINT error");
        return -1;
    }
    //忽略PIPE信号
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGPIPE error");
        return -1;
    }
    return 0;
}

string BaseClientMock::RoleName()
{
    return "client";
}

void BaseClientMock::SetStopFlag(int)
{
    GLOG(IM_INFO, "child ready to exit...");
    s_stopFlag = true;
}

void BaseClientMock::SetInfo(int num, int sec, int pause_ms, 
        const string& qID, const string& qaID)
{
    m_num = num;
    m_sec = sec;
    m_pause_ms = pause_ms;
    m_qID = qID;
    m_qaID = qaID;
}

int BaseClientMock::Loop()
{
    int ret = 0;
    int tmp = 0;
    if (m_sec > 0)
    {
        for (time_t b = time(NULL); ! s_stopFlag && time(NULL) - b < m_sec;)
        {
            if ((tmp = SendDat()) != 0)
            {
                ret = tmp;
            }
        }
    }
    else if (m_num > 0)
    {//需要发送的次数
        for (int i = 0; ! s_stopFlag && i < m_num; ++i)
        {
            if ((tmp = SendDat()) != 0)
            {
                ret = tmp;
            }
        }
    }
    return ret;
}

int BaseClientMock::SendDat()
{
    int ret = 0;
    int tmp = 0;
    EList<QA*>& v = m_dat->GetQA();
    for (QA** it = v.begin();
            ! s_stopFlag && ! v.end(); it = v.next())
    {
        if (! m_qaID.empty())
        {
            if (m_qaID != (*it)->GetDebugID())
            {
                GLOG(IM_DEBUG, "skip %s, qa id != %s", (*it)->GetDebugID(), m_qaID.c_str());
                continue;
            }
        }
        RRMessage req;
        if ((*it)->Query(&req, m_qID))
        {
            long inBytes = 0, outBytes = 0;
            double spentTimeMs = 0.0;
            if (0 != (tmp = Send(*it, &req, inBytes, outBytes, spentTimeMs)))
            {
                ret = tmp;
            }
            if (outBytes != 0)
            {
                m_mmapProcInfo->AddOneOut(outBytes);
                m_mmapProcInfo->AddTime(spentTimeMs);
            }
            if (inBytes != 0)
            {
                m_mmapProcInfo->AddOneIn(inBytes);
            }
            GLOG(IM_DEBUG, "realinfo: in(%ld), out(%ld), time ms(%lf)",
                    inBytes, outBytes, spentTimeMs);
        }
        if (m_pause_ms > 0)
        {
            usleep(m_pause_ms*1000);
        }
    }
    return ret;
}

int BaseClientMock::Send(QA* qa, RRMessage* req, 
        long& inBytes, long& outBytes, double& spentTimeMs)
{
    return 0;
}

void BaseClientMock::SetWorkers(int w)
{
    m_workers = w;
}

void BaseClientMock::StopAllWorkers()
{
    for (set<int>::iterator it = m_wpids.begin(); it != m_wpids.end(); ++it)
    {
        kill(*it, SIGINT);
    }
}
