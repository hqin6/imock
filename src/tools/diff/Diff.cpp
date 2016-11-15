#include "imock-interface.h"
#include "DiffInfo.h"
#include "INIConf.h"
#include "Log.h"
#include "Dat.h"
#include "Str.h"
#include <sstream>
#include <iostream>

using namespace std;
using namespace log4cpp;

static void usage(const char* programName)
{
    printf("\n");
    printf("Usage: %s [OPTION] \n", programName);
    printf("\n");
    printf("  -c <config file>  [required] the config file\n");
    printf("  -l <log level>    [optional] can use: debug|info|error, default:info\n");
    printf("  -b                [optional] if find no match, break immediately\n");
    printf("  -o <result file>  [optional] store the match result\n");
    printf("  -h                [optional] show the help info\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c imock-diff.conf -l error\n", programName);
}

vector<DiffInfo*> g_diffInfo;
Dat* g_matchDat = NULL;

static bool LoadLogs(const string& val, void* data, uint64_t o)
{
    if (val.empty())
    {
        return false;
    }
    // 获取match属性
    string datXml = "";
    if (! g_iniConf->Get("", "match", ISet::Str, &datXml))
    {
        GLOG(IM_ERROR, "can't find cmd<match>\n");
        return false;
    }
    if (g_matchDat)
    {
        delete g_matchDat;
        g_matchDat = NULL;
    }
    g_matchDat = new Dat;
    g_matchDat->Load(datXml.c_str(), NULL, "", "");
    // 获取第一个q节点
    EList<QA*>& qa = g_matchDat->GetQA();
    if (0 == qa.size())
    {
        GLOG(IM_ERROR, "no qa in %s", datXml.c_str());
        return false;
    }
    QA** pqa = qa.begin();
    vector<DatNodeRoot*>& vq = (*pqa)->GetQ();
    if (vq.empty())
    {
        GLOG(IM_ERROR, "no q in %s", datXml.c_str());
        return false;
    }
    DatNode* mdn = *vq.begin();

    // 读取域area
    stringstream ss(val);
    string area = "";
    while (getline(ss, area, ','))
    {
        Str::Trim(area);
        if (area.empty())
        {
            continue;
        }
        // 获取area对应的配置
        DiffInfo* di = new DiffInfo(area);
        if (! di->Init())
        {
            delete di;
            return false;
        }
        if (! di->ParseFile(mdn))
        {
            delete di;
            return false;
        }
        g_diffInfo.push_back(di);
    }
    return true;
}

int main(int argc, char* argv[])
{
    const char* confFile = NULL;
    const char* logLevel = "info";
    string resultFile = "";
    bool brk = false;

    while (true)
    {
        int c = getopt(argc, argv, "c:l:hbo:");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'o':
                resultFile = optarg;
                break;
            case 'b':
                brk = true;
                break;
            case 'c':
                confFile = optarg;
                break;
            case 'l':
                logLevel = optarg;
                break;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    //如果没有指定config
    if (NULL == confFile)
    {
        printf("config file must be given\n");
        return -1;
    }

    // 加载日志
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
        map<string, Priority::Value>::iterator it = mp.find(logLevel);
        if (mp.end() == it)
        {
            printf("unrecognized log level:%s\n", logLevel);
            return false;
        }
        v = it->second;
    }

    g_log = new Logger("/dev/stdout");
    g_log->SetLevel(v);
    if (! g_log->Load())
    {
        return false;
    }

    // 加载配置
    g_iniConf = new INIConf();
    if (! g_iniConf->Load(confFile))
    {
        return -1;
    }

    // 读取所有需要对比的域
    if (! g_iniConf->Get("", "diff", LoadLogs, NULL))
    {
        return -1;
    }

    // 比对
    if (g_diffInfo.size() < 2) 
    {
        GLOG(IM_DEBUG, "diff area less than 2, no need diff");
        return 0;
    }

    DiffInfo* baseDI = g_diffInfo[0];
    string resultStr = "";
    int ret = 0;
    for (int i = 1; i < (int)g_diffInfo.size(); ++i)
    {
        if (! baseDI->DoDiff(g_diffInfo[i], brk, resultStr))
        {
            ret = -1;
            if (brk)
            {
                break;
            }
        }
    }
    FILE* fp = NULL;
    if (resultFile.empty())
    {
        cout << resultStr;
    }
    else
    {
        fp = fopen(resultFile.c_str(), "w");
        if (NULL == fp)
        {
            printf("open %s failed.", resultFile.c_str());
            return -1;
        }
        fwrite(resultStr.c_str(), resultStr.size(), 1, fp);
        fclose(fp);
    }
    return ret;
}
