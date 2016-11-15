#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include "INIConf.h"
#include "Log.h"
#include "MockClientFactory.h"
#include "Str.h"
#include "Daemon.h"
#include "ProcTitle.h"
#include "google/protobuf/message.h"
#include "BaseClientMock.h"

using namespace std;
using namespace log4cpp;

Logger* g_log = NULL;

void usage(const char* programName)
{
    printf("\n");
    printf("Usage: %s [OPTION] \n", programName);
    printf("Send query to server and get answer from server \n");
    printf("\n");
    printf("  -a <area name>      the area will be used in config file.\n");
    printf("  -c <config file>    default:/home/a/imock/conf/client/imock-client.conf\n");
    printf("  -n <send num>       the number of sending all query for a worker.\n");
    printf("  -s <send seconds>   the seconds for client workers run.\n");
    printf("  -w <workers>        the number of client workers.\n");
    printf("  -l <log level>      crit, error, warn, notice, info, debug. default:info\n");
    printf("  -p <msec>           pause milliseconds intervals\n");
    printf("  -I <qa id>          the qa id which need send\n");
    printf("  -i <q id>           the q id which need send in qa\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c imock-client.conf -a acx\n", programName);
    printf("  %s -c imock-client.conf -a acx -w 8\n", programName);
}

static BaseClientMock* g_bm = NULL;

static void StopAllChildren(int no)
{
    if (g_bm)
    {
        g_bm->StopAllWorkers();
    }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    ::google::protobuf::SetLogHandler(NULL);

    string config, area;
    int worker = -1;
    int num = 1;
    int seconds = 0;
    int pause_ms = 0;
    string loglevel = "info";
    string qaID = "";
    string qID = "";

    while (true)
    {
        int c = getopt(argc, argv, "a:c:n:s:w:l:p:I:i:");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'a':
                area = optarg;
                break;
            case 'c':
                config = optarg;
                break;
            case 'w':
                worker = atoi(optarg);
                break;
            case 'n':
                num = atoi(optarg);
                break;
            case 's':
                seconds = atoi(optarg);
                break;
            case 'l':
                loglevel = optarg;
                break;
            case 'p':
                pause_ms = atoi(optarg);
                break;
            case 'I':
                qaID = optarg;
                break;
            case 'i':
                qID = optarg;
                break;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    if (config.empty())
    {
        config = "/home/a/imock/conf/client/imock-client.conf";
    }
    if (area.empty())
    {
        usage(argv[0]);
        return -1;
    }
    if (num < 1 && seconds < 1)
    {
        usage(argv[0]);
        return -1;
    }
    Priority::Value v;
    {
        map<string, Priority::Value> mp;
        mp["emerg" ] = IM_EMERG;
        mp["fatal" ] = IM_FATAL;
        mp["alert" ] = IM_ALERT;
        mp["crit"  ] = IM_CRIT;
        mp["error" ] = IM_ERROR;
        mp["warn"  ] = IM_WARN;
        mp["notice"] = IM_NOTICE;
        mp["info"  ] = IM_INFO;
        mp["debug" ] = IM_DEBUG;
        map<string, Priority::Value>::iterator it = mp.find(loglevel);
        if (mp.end() == it)
        {
            printf("unrecognized log level:%s", loglevel.c_str());
            return -1;
        }
        v = it->second;
    }
    //设置日志为控制台输出
    g_log = new Logger("/dev/stdout");
    g_log->SetLevel(v);
    if (! g_log->Load())
    {
        return -1;
    }
    
    g_iniConf = new INIConf();
    if (! g_iniConf->Load(config))
    {
        return -1;
    }
    //设置进程名的前置条件
    ProcTitle::SaveEnv(argv);
    ProcTitle::Set("imock: master client " + string(ProcTitle::GetMainArgv()));

    MockClientFactory mf;
    g_bm = (BaseClientMock*)mf.GetMock(area);
    if (NULL == g_bm)
    {
        return -1;
    }
    if (! g_bm->Init())
    {
        delete g_bm;
        g_bm = NULL;
        return -1;
    }
    //使用命令行参数覆盖
    if (worker > 0)
    {
        g_bm->SetWorkers(worker);
    }
    g_bm->SetInfo(num, seconds, pause_ms, qID, qaID);
    g_bm->Run();
    //设置忽略所有信号
    struct sigaction sa;
    bzero(&sa, sizeof(struct sigaction));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = StopAllChildren;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGINT error");
    }
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGTERM error");
    }
    /*if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        GLOG(IM_ERROR, "reg SIGCHLD error");
    }*/
    GLOG(IM_INFO, "waiting...");
    //等待所有子进程退出
    int status = 0;
    while (true)
    {
        int tmp = 0;
        int pid = wait(&tmp);
        if (tmp != 0)
        {
            status = tmp;
        }
        if (pid > 0)
        {
            GLOG(IM_INFO, "child process %d exits.", pid);
        }
        else if (-1 == pid && ECHILD == errno)
        {
            GLOG(IM_INFO, "all children exits, last status:%d", status);
            break;
        }
    }
    delete g_bm;
    g_bm = NULL;
    return status != 0 ? 1 : 0;
}
