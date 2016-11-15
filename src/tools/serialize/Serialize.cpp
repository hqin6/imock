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
    printf("  -f <fmt file>  [required] the format file used to serialize the data\n");
    printf("  -d <dat file>  [optional] the data need to be serialized \n");
    printf("  -l <log level> [optional] can use: debug|info|error, default:info\n");
    printf("  -o <file>      [optional] the serialized result\n");
    printf("  -a             [optional] use a node or not\n");
    printf("  -i             [optional] use fmtId(fid) to parse\n");
    printf("  -h             [optional] show the help info\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -f fmt.xml -d dat.xml -l error\n", programName);
}

int main(int argc, char* argv[])
{
    const char* fmtFile = NULL;
    const char* datFile = NULL;
    const char* loglevel = "info";
    string fid = "";
    bool isq = true;
    string resultFile = "";

    while (true)
    {
        int c = getopt(argc, argv, "f:d:l:hai:o:");
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'o':
                resultFile = optarg;
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

	vector<IQA> dst;

	if (iif.Serialize(dst))
	{
		string result = "";
		for (vector<IQA>::iterator it = dst.begin(); it != dst.end(); ++it)
		{
			if (isq)// 序列化q节点
			{
				result += *(it->q.begin());
			}
			else 
			{
				result += *(it->a.begin());
			}
		}
		// 写文件
		FILE* fp = NULL;
		if (resultFile.empty())
		{
			cout << result;
		}
		else
		{
			fp = fopen(resultFile.c_str(), "w");
			if (NULL == fp)
			{
				printf("open %s failed.", resultFile.c_str());
				return -1;
			}
			fwrite(result.c_str(), result.size(), 1, fp);
			fclose(fp);
		}

		return 0;
	}
	printf("serialized failed.\n");
    return 1;
}
