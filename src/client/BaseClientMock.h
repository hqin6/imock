/**
 * 文件名称：BaseClientMock.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _BASECLIENTMOCK_H_
#define _BASECLIENTMOCK_H_

#include "BaseMock.h"
#include "Fmt.h"
#include "Dat.h"

class BaseClientMock : public BaseMock
{
    public:
        BaseClientMock(const std::string& name = "");
        virtual ~BaseClientMock();
        void SetWorkers(int w);
        void SetInfo(int num, int sec, int pause_ms, 
                const std::string& qID, const std::string& qaID);
        virtual bool Init();
        void StopAllWorkers();
    protected:
        //注册信号处理函数
        int AfterFork();
        virtual int Loop();
        virtual int Send(QA* qa, RRMessage* msg);
    private:
        //返回client角色固有的进程名
        std::string RoleName();
        int SendDat();
    private:
        //退出信号处理函数
        static void SetStopFlag(int);
    protected:
        //标记服务是否应该停止了
        static bool s_stopFlag;
    protected:
        Fmt* m_fmt;
        Dat* m_dat;
        int m_num;
        int m_sec;
        int m_pause_ms;// 每次发送后的暂停
        int m_timeout;
        std::string m_qID;
        std::string m_qaID;
};

#endif
