/**
 * 文件名称：BaseServerMock.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _BASESERVERMOCK_H_
#define _BASESERVERMOCK_H_

#include "BaseMock.h"
#include "Fmt.h"
#include "Dat.h"

class BaseServerMock : public BaseMock
{
    public:
        BaseServerMock(const std::string& name = "");
        virtual ~BaseServerMock();
    protected:
        //注册信号处理函数
        int AfterFork();
        virtual bool Init();
    private:
        //返回server角色固有的进程名
        std::string RoleName();
    private:
        //退出信号处理函数
        static void SetStopFlag(int);
    protected:
        //标记服务是否应该停止了
        static bool s_stopFlag;
    protected:
        Fmt* m_fmt;
        Dat* m_dat;
        //是否缓存dat数据
        bool m_cachedDat;
};

#endif
