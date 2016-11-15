/**
 * 文件名称：Daemon.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2013.06.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DAEMON_H_
#define _DAEMON_H_

class Daemon
{
    public:
        //fork进程，并将pid写入pidFile中
        static int Start(const char* pidFile);
        static int Stop(const char* pidFile);
    private:
        static int ReadPidFile(const char* pidFile);
        static int WritePidFile(const char* pidFile);
};


#endif
