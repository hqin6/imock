/**
 * 文件名称：HttpServerMock.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.06
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _HTTPSERVERMOCK_H_
#define _HTTPSERVERMOCK_H_

#include "BaseServerMock.h"
#include "event2/event.h"
#include "event2/http.h"

struct TimerInfo
{
    struct evhttp_request *req;
    RRMessage* res;
    struct timeval recvMoment;
};

class HttpServerMock : public BaseServerMock
{
    public:
        //调用父类的构造
        HttpServerMock(const std::string& name = "", bool useSSL = false);

        //析构
        virtual ~HttpServerMock();

        //初始化接口，读取配置
        //设置信号停止函数
        bool Init();

    private:
        //循环
        virtual int Loop();

        //bind port
        virtual int BeforeFork();

    private:
        //超时回调函数，检查是否需要退出
        static void InterruptLoop(int fd, short kind, void* userp);
        //超时回调函数，用于模拟延时
        static void TimerCallBack(int fd, short kind, void* userp);
        //event的回调函数，处理每个请求
        static void Process(struct evhttp_request *req, void *arg);
        static void Send(struct evhttp_request* req, 
                RRMessage* res, 
                struct timeval& recvMoment,
                void* arg);

    private:
        int m_port;
        struct event_base* m_base;
        struct evhttp*     m_http;
        struct event*      m_tm;
        //用于接收到信号后退出
        struct event*      m_exitEv;
        struct evhttp_bound_socket* m_handle;
        bool m_useSSL; // 是否使用ssl,即是否是https
        std::string m_sslCert;    // ssl_certificate
        std::string m_sslCertKey; // ssl_certificate_key


};


#endif
