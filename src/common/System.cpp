#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "System.h"
#include "Log.h"
#include "popenv.h"

using namespace std;

System::System()
{
    m_argc = 0;
    //Add("sh");
    //Add("-c");
}

const string System::Cmd()
{
    if (m_argc < 1)
    {
        return "";
    }
    if (m_argc >= (int)sizeof(m_argv))
    {
        return "";
    }
    Add(NULL);

    char buf[1024 * 1024];
    FILE* pp = popenv(m_argv, (char*)"r");
    if (pp == NULL)
    {
        GLOG(IM_ERROR, "exec cmd failed: %s, error:%s", m_argv[0], strerror(errno));
        return "";
    }

    int len = fread(buf, 1, sizeof(buf) - 1, pp);
    GLOG(IM_DEBUG, "exec cmd %s, read len:%d", m_argv[0], len);
    if (len < 0)
    {
        pclosev(pp);
        return "";
    }

    buf[len] = '\0';
    GLOG(IM_DEBUG, "system return:%s", buf);

    pclosev(pp);
    return string(buf, len);
}
#ifdef _JUST_TEST
#include <iostream>
Logger* g_log = NULL;
int main()
{
    System s;
    s.Add("echo");
    s.Add("0\0aaa");
    s.Add("1");
    s.Add("\"1\"");
    cout << s.Cmd();
    return 0;
}
#endif
