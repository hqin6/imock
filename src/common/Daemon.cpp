#include "Daemon.h"
#include "Log.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

int Daemon::Start(const char* pidFile)
{
    int pid = ReadPidFile(pidFile);
    if (pid > 0 && 0 == kill(pid, 0))
    {//有另一个已经存在的进程
        GLOG(IM_ERROR, "another pid is running, pid=%d", pid);
        return -1;
    }
    //fork两次
    for (int i = 0; i < 2; ++i)
    {
        switch (fork())
        {
            case -1: //error
                GLOG(IM_ERROR, "call fork() error");
                return -1;
            case 0://子进程
                break;
            default://父进程退出
                exit(0);
        }
        setsid();
    }
    //写pid文件
    if (WritePidFile(pidFile) != 0)
    {
        return -1;
    }

    int fd = open("/dev/null", O_RDWR);
    if (-1 == fd)
    {
        GLOG(IM_ERROR, "call open() failed");
        return -1;
    }
    //0重定向到/dev/null
    if (dup2(fd, 0) == -1)
    {//stdin
        GLOG(IM_ERROR, "call dup2() failed");
        return -1;
    }
    //1重定向到/dev/null
    if (dup2(fd, 1) == -1)
    {//stdout
        GLOG(IM_ERROR, "call dup2() failed");
        return -1;
    }
    //关闭fd
    if (fd > 2) 
    {
        if (close(fd) == -1) 
        {
            GLOG(IM_ERROR, "call close(%d) error", fd);
            return -1;
        }
    }
    return 0;
}

int Daemon::Stop(const char* pidFile)
{
    /*这里需要和main函数注册的信号一致*/
    int pid = ReadPidFile(pidFile);
    if (pid < 0)
    {
        GLOG(IM_ERROR, "read %s failed", pidFile);
        return pid;
    }
    if (0 != kill(-pid, SIGINT))
    {
        GLOG(IM_ERROR, "call kill() error");
    }
    return 0;
}

int Daemon::ReadPidFile(const char* pidFile)
{
    FILE* fp = fopen(pidFile, "r");
    if (NULL == fp)
    {
        return -1;
    }
    int pid = 0;
    int ret = fscanf(fp, "%d", &pid);
    if (EOF == ret || ferror(fp))
    {
        return -1;
    }
    fclose(fp);
    return pid;
}

int Daemon::WritePidFile(const char* pidFile)
{
    FILE* fp = fopen(pidFile, "w");
    if (NULL == fp)
    {
        GLOG(IM_ERROR, "open %s error", pidFile);
        return -1;
    }
    fprintf(fp, "%d", getpid());
    fclose(fp);
    return 0;
}
