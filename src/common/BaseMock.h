/**
 * 文件名称：BaseMock.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.02
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _BASEMOCK_H_
#define _BASEMOCK_H_

#include <string>
#include <set>
#include <stdint.h>

#define CACHED_FMT 0x01
#define CACHED_DAT 0x02

#define MODE_RECV 0x01
#define MODE_SEND 0x02

#include "Alloc.h"

class BaseMock : public Alloc
{
    public:
        //任何mock，必须有一个名字，用来初始化配置和进程名
        BaseMock(const std::string& name = ""); 
        virtual ~BaseMock();

        //设置mock所对应的名字，用来初始化配置和进程名
        void SetName(const std::string& name);

        //成功返回0，失败返回-1
        //确保调用时GLOG可用
        virtual bool Init();

        //负责fork进程，并修改子进程的名字
        //子进程以imock [w][s|c][m_name]命名
        virtual void Run();

        //写入原始输入消息
        //支持trunc写入：即只保留最近的一个输入消息
        //支持append写入：即保留所有的输入消息
        void WriteMsg(const std::string& msg);

    protected:
        //在子类可继承该方法，在fork前执行操作
        //返回非0，即不再进行fork，直接退出
        virtual int BeforeFork(); 

        //在子类可继承该方法，在fork后执行操作
        //返回非0，直接退出
        virtual int AfterFork();

        //接口，需要子类实现
        virtual int Loop() = 0;

        //设置进程名字
        virtual std::string RoleName() = 0;

    private:
        //读取message配置，包括文件名和mode
        static bool SetMsgFile(const std::string& s, void* d, uint64_t o);
        // 读取mode配置，RS/R/S
        static bool SetMode(const std::string& s, void* d, uint64_t o);

    protected:
        //mock名字，以及配置的area
        std::string m_name;
        //fmt数据文件
        std::string m_fmtFile;
        //dat数据文件
        std::string m_datFile;
        //保存输入数据
        std::string m_msgFile;
        //输入数据记录到文件的模式，即fopen的mode
        //默认值为w
        std::string m_msgFileMode;
        //需要fork的进程数
        int m_workers;
        //所有worker的pid进程号
        std::set<int> m_wpids;
        // 发送接收模式，第一位表示R(recv)，第二位表示S(send)
        int m_mode;
    private:
};


#endif
