#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <sys/mman.h>
#include "INIConf.h"
#include "Log.h"
#include "MockServerFactory.h"
#include "Str.h"
#include "Daemon.h"
#include "ProcTitle.h"
#include "google/protobuf/message.h"

using namespace std;
using namespace log4cpp;

/*---全局指针，包括INI和LOG---*/
Logger* g_log = NULL;

//帮助信息
static void usage(const char* programName)
{
    printf("\n");
    printf("Usage: %s [OPTION] \n", programName);
    printf("\n");
    printf("  -c <config file>  default: /home/a/imock/conf/server/imock-server.conf\n");
    printf("  -s <start|stop>   default: start\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -s start\n", programName);
    printf("  %s -c imock-server.conf -s start\n", programName);
}

struct MmapProcInfo 
{
    ProcInfo* pi;
    BaseMock* bm;
    int size;
};

static bool LoadMocks(const string& val, void* data, uint64_t o)
{
    if (val.empty())
    {
        return false;
    }
    MockServerFactory mf;
    stringstream ss(val);
    string area = "";
    vector<BaseMock*> allm;
    //按照逗号分隔多个mock(area)
    while (getline(ss, area, ','))
    {
        Str::Trim(area);
        if (area.empty())
        {
            continue;
        }
        BaseMock* bm = mf.GetMock(area);
        if (NULL == bm)
        {
            return false;
        }
        if (! bm->Init())
        {
            return false;
        }
        allm.push_back(bm);
        GLOG(IM_DEBUG, "Run %s", area.c_str());
    }
    vector<MmapProcInfo> mmapPIs;
    for (vector<BaseMock*>::iterator it = allm.begin(); 
            it != allm.end(); ++it)
    {
        MmapProcInfo mmapPI;
        mmapPI.size = (*it)->GetWorkers() * sizeof(ProcInfo);
        mmapPI.pi = (ProcInfo*)mmap(NULL, 
                mmapPI.size, PROT_READ|PROT_WRITE, 
                MAP_SHARED|MAP_ANONYMOUS, -1, 0);
        mmapPIs.push_back(mmapPI);
        mmapPI.bm = *it;
        (*it)->Run(mmapPI.pi);
    }

    //设置忽略所有信号
    struct sigaction sa;
    bzero(&sa, sizeof(struct sigaction));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGINT error");
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGTERM error");
    }
    /*
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGCHLD error");
    }*/
    GLOG(IM_DEBUG, "waiting...");
    //等待所有子进程退出
    while (true)
    {
        int pid = wait(NULL);
        if (pid > 0)
        {
            GLOG(IM_INFO, "child process %d exits.", pid);
        }
        else if (-1 == pid && ECHILD == errno)
        {
            GLOG(IM_INFO, "all children exits!");
            break;
        }
    }
    for (vector<MmapProcInfo>::iterator it = mmapPIs.begin();
            it != mmapPIs.end(); ++it)
    {
        GLOG(IM_INFO, "[%s] %s", it->bm->GetName().c_str(),
                ProcInfo::DebugString(it->pi, it->size / sizeof(ProcInfo)).c_str());
        munmap(it->pi, it->size);
    }
    return true;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    //设置proto不打印错误日志
    ::google::protobuf::SetLogHandler(NULL);
    //读取配置
    string conf, cmd = "start";
    while (true)
    {
        int c = getopt(argc, argv, "c:s:h");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'c':
                conf = optarg;
                break;
            case 's':
                cmd = optarg;
                break;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    //如果没有指明配置，则使用默认配置。
    if (conf.empty())
    {
        conf = "/home/a/imock/conf/server/imock-server.conf";
    }
    //读取配置文件前，其日志应该是标准输出
    g_log = new Logger("/dev/stdout");
    if (! g_log->Load())
    {
        return -1;
    }
    //读取配置
    g_iniConf = new INIConf();
    if (! g_iniConf->Load(conf))
    {
        return -1;
    }
    string pid;
    if (! g_iniConf->Get("", "pid_file", ISet::Str, &pid, "/tmp/imock-server.pid"))
    {
        return -1;
    }
    if ("start" == cmd)
    {
        if (0 != Daemon::Start(pid.c_str()))
        {
            return -1;
        }
    }
    else if ("stop" == cmd)
    {
        int ret = Daemon::Stop(pid.c_str());
        if (ret != 0)
        {
            GLOG(IM_ERROR, "no process to be stopped");
        }
        return ret;
    }
    else
    {
        GLOG(IM_ERROR, "no support cmd:%s", cmd.c_str());
        return -1;
    }
    //设置进程名
    ProcTitle::SaveEnv(argv);
    ProcTitle::Set("imock: master server " + string(ProcTitle::GetMainArgv()));
    //重新设定日志文件
    delete g_log;
    g_log = new Logger;
    if (! g_iniConf->Get("", "log", LoadLog, NULL))
    {
        return -1;
    }
    GLOG(IM_DEBUG, "Load log ok!");
    //读取不同的mock
    if (! g_iniConf->Get("", "mocks", LoadMocks, NULL))
    {
        return -1;
    }
    return 0;
}
