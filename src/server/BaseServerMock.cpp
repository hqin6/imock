#include "BaseServerMock.h"
#include <string>
#include <signal.h>
#include "Log.h"
#include "INIConf.h"
using namespace std;

bool BaseServerMock::s_stopFlag = false;

BaseServerMock::BaseServerMock(const string& name) : 
    BaseMock(name),
    m_fmt(NULL),
    m_dat(NULL)

{
}

BaseServerMock::~BaseServerMock()
{
    delete m_fmt;
    delete m_dat;
}

bool BaseServerMock::Init()
{
    if (! BaseMock::Init())
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
    if (! g_iniConf->Get(m_name, "cache", ISet::Flag, &m_cachedDat, "off"))
    {
        return false;
    }
    return true;
}

int BaseServerMock::AfterFork()
{
    if (0 != BaseMock::AfterFork())
    {
        return -1;
    }
    struct sigaction sa;
    bzero(&sa, sizeof(struct sigaction));
    //增加退出机制
    sa.sa_handler = BaseServerMock::SetStopFlag;
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

string BaseServerMock::RoleName()
{
    return "server";
}

void BaseServerMock::SetStopFlag(int)
{
    GLOG(IM_INFO, "child ready to exit...");
    s_stopFlag = true;
}
