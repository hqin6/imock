#include <string.h>
#include <stdlib.h>
#include "ProcTitle.h"

using namespace std;

extern char **environ;

char** ProcTitle::s_osArgv = NULL;
char* ProcTitle::s_osArgvLast = NULL;
char* ProcTitle::s_mainArgv = NULL;

int ProcTitle::SaveEnv(char* const* argv)
{
    s_osArgv = (char**)argv;
    //将environ赋值一份出来, s_osArgvLast是原有的空间
    int size = 0;
    for (int i = 0; environ[i]; ++i)
    {
        size += strlen(environ[i]) + 1;
    }
    char* p = (char*)malloc(size);
    s_osArgvLast = s_osArgv[0];
    int totalLen = 0;
    for (int i = 0; s_osArgv[i]; ++i)
    {
        int len = strlen(s_osArgv[i]);
        if (s_osArgvLast == s_osArgv[i])
        {
            s_osArgvLast = s_osArgv[i] + len + 1;
        }
        totalLen += len + 1;
    }
    s_mainArgv = (char*)malloc(totalLen + 1);
    char* tmp = s_mainArgv;
    for (int i = 0; s_osArgv[i]; ++i)
    {
        strcpy(tmp, s_osArgv[i]);
        tmp += strlen(s_osArgv[i]);
        *tmp++ = ' ';
    }
    *tmp = '\0';
    for (int i = 0; environ[i]; ++i)
    {
        if (s_osArgvLast == environ[i])
        {
            size = strlen(environ[i]) + 1;
            s_osArgvLast = environ[i] + size;
            strncpy(p, environ[i], size);
            environ[i] = p;
            p += size;
        }
    }
    s_osArgvLast--;//预留一个\0

    return 0;
}

char* ProcTitle::GetMainArgv()
{
    return s_mainArgv;
}

void ProcTitle::Set(const string& pname)
{
    if (pname.empty())
    {
        return ;
    }
    s_osArgv[1] = NULL;
    strncpy(s_osArgv[0], pname.c_str(), s_osArgvLast - s_osArgv[0]);
}

