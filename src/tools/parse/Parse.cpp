#include "imock-interface.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

static void usage(const char* programName)
{
    printf("\n");
    printf("Usage: %s [OPTION] \n", programName);
    printf("\n");
    printf("  -f <fmt file>  [required] the format file used to parse the stdin\n");
    printf("  -d <dat file>  [optional] the parsed result is matched the dat file or not\n");
    printf("  -l <log level> [optional] can use: debug|info|error, default:info\n");
    printf("  -o <xml file>  [optional] the result xml file exported\n");
    printf("  -a             [optional] use a node or not\n");
    printf("  -i             [optional] use fmtId(fid) to parse\n");
    printf("  -h             [optional] show the help info\n");
    printf("  -e             [optional] add one qa \"<dat><qa></qa></dat>\" for extra\n");
    printf("  -E             [optional] add multi qa \"<dat><qa></qa></dat>\" for extra\n");
    printf("\n");
    printf("Examples:\n");
    printf("  tail -1 log.txt | %s -f fmt.xml -d dat.xml -l error\n", programName);
}

int main(int argc, char* argv[])
{
    const char* fmtFile = NULL;
    const char* datFile = NULL;
    const char* loglevel = "info";
    string fid = "";
    bool isq = true;
    string objxml = "";
    int extra = 0;

    while (true)
    {
        int c = getopt(argc, argv, "f:d:l:hai:o:eE");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'o':
                objxml = optarg;
                break;
            case 'f':
                fmtFile = optarg;
                break;
            case 'd':
                datFile = optarg;
                break;
            case 'l':
                loglevel = optarg;
                break;
            case 'a':
                isq = false;
                break;
            case 'i':
                fid = optarg;
                break;
            case 'e':
                extra = 1;
                break;
            case 'E':
                extra = 2;
                break;
            default:
                usage(argv[0]);
                return -1;
        }
    }
    //如果没有指定fmt
    if (NULL == fmtFile)
    {
        printf("fmt file must be given\n");
        return -1;
    }

    ImockInterface iif;

    if (! iif.Init(loglevel))
    {
        printf("init failed.\n");
        return -1;
    }

    if (! iif.LoadFmtFile(fmtFile))
    {
        return 1;
    }

    // 读取标准输入
    ostringstream tmp;
    tmp << cin.rdbuf();
    string buf = tmp.str();

    if (NULL != datFile)
    {
        if (isq)
        {
            if (! iif.LoadDatFile(datFile, fid, ""))
            {
                return 1;
            }
        }
        else
        {
            if (! iif.LoadDatFile(datFile, "", fid))
            {
                return 1;
            }
        }

        string xml;
        if (! iif.PMatch(buf, xml, isq))
        {
            return 1;
        }
        return 0;
    }
    else
    {// 只解析不匹配
        string xml;
        string sepb = "", sepe = "";
        if (extra == 2) 
        {
            sepb = "<qa>";
            sepe = "</qa>";
        }
        if (iif.Parse(buf, xml, isq, "", NULL, sepb, sepe))
        {
            if (extra == 1) 
            {
                xml = "<dat><qa>" + xml + "</qa></dat>";
            }
            else if (extra == 2)
            {
                xml = "<dat>" + xml + "</dat>";
            }
            // 写文件
            FILE* fp = NULL;
            if (objxml.empty())
            {
                cout << xml;
            }
            else
            {
                fp = fopen(objxml.c_str(), "w");
                if (NULL == fp)
                {
                    printf("open %s failed.", objxml.c_str());
                    return -1;
                }
                fwrite(xml.c_str(), xml.size(), 1, fp);
                fclose(fp);
            }

            return 0;
        }
        printf("len=%d, parse failed.\n", (int)buf.size());
    }
    return 1;
}
