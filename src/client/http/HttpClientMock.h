/**
 * 文件名称：HttpClientMock.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.06
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _HTTPCLIENTMOCK_H_
#define _HTTPCLIENTMOCK_H_

#include "BaseClientMock.h"
#include "curl/curl.h"

class HttpClientMock : public BaseClientMock
{
    public:
        //调用父类的构造
        HttpClientMock(const std::string& name = "");

        //析构
        virtual ~HttpClientMock();

        //初始化接口，读取配置
        //设置信号停止函数
        bool Init();

    private:
        int Send(QA* qa, RRMessage* req, long& inBytes, long& outBytes, double& spentTimeMs);
        void SetPostFlag(RRMessage* req);
        void SetUrl(RRMessage* req);
        curl_slist* SetHttpHead(RRMessage* req);
        static size_t WriteData(void* ptr, size_t size, size_t nmemb, void* userData);

    private:
        std::string m_url;
        CURL* m_curl;
        char* m_response;
        char* m_responseHead;
        static int s_responseMaxLen;
};


#endif
