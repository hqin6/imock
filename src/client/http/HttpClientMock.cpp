#include "HttpClientMock.h"
#include "Log.h"
#include "INIConf.h"
#include "Str.h"

using namespace std;

int HttpClientMock::s_responseMaxLen = 20480;

size_t HttpClientMock::WriteData(void* ptr, size_t size, size_t nmemb, void* userData)
{
    char* p = (char*)userData;
    int oldLen = *(int*)p;
    int addLen = size * nmemb;
    if (oldLen + addLen > s_responseMaxLen)
    {
        return 0;
    }
    memcpy(p + sizeof(int) + oldLen, ptr, addLen);
    *(int*)p = addLen + oldLen;
    return addLen;
}

HttpClientMock::HttpClientMock(const string& name)
    : BaseClientMock(name), 
    m_curl(NULL), m_response(NULL), m_responseHead(NULL)
{
}

HttpClientMock::~HttpClientMock()
{
    if (m_curl)
    {
        curl_easy_cleanup(m_curl);
    }
    delete [] m_response;
    delete [] m_responseHead;
}

bool HttpClientMock::Init()
{
    if (! BaseClientMock::Init())
    {
        return false;
    }
    Command c[] = {
        { m_name.c_str(), "url",     ISet::Str, (uint64_t)&m_url, NULL },
        { m_name.c_str(), "resp_max_len",     ISet::Int, (uint64_t)&s_responseMaxLen, "20480" },
        { NULL,           NULL,      NULL,      0          , NULL }
    };
    if (! g_iniConf->Get(c, 0))
    {   
        return false;
    }
    m_response = new char[s_responseMaxLen+ sizeof(int)];
    m_responseHead = new char[s_responseMaxLen + sizeof(int)];
    return true;
}

void HttpClientMock::SetPostFlag(RRMessage* req)
{
    //设置method
    string method = req->GetMethod();
    const string& body = req->GetBody();
    if (method.empty())
    {//没有设置method
        if (! body.empty())
        {//POST,如果body不为空，则默认使用post发送
            method = "POST";
        }
        else 
        {
            method = "GET";
        }
    }
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (! body.empty())
    {//如果指定了post，不管body如何，均使用post
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, body.size());
    }
    #ifdef CURLOPT_ACCEPT_ENCODING
    curl_easy_setopt(m_curl, CURLOPT_ACCEPT_ENCODING, NULL); 
    #endif
    int m = req->GetHttpVersionMinor();
    if (m == 0)
    {// 只支持0/1
        curl_easy_setopt(m_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    }
    else if (m == 1)
    {
        curl_easy_setopt(m_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    }
}

void HttpClientMock::SetUrl(RRMessage* req)
{
    string url = m_url;
    const string& uriPath = req->GetUriPath();
    if (! uriPath.empty())
    {
        url += uriPath;
    }
    EList<pair<string, string> >& vh = req->GetHttpUriQuery();
    string split = "";
    string kv;
    for (pair<string, string>* it = vh.begin();
            ! vh.end(); it = vh.next())
    {
        kv += split + it->first + "=" + it->second;
        split = "&";
    }
    if (! kv.empty())
    {
        if (string::npos == url.find('?'))
        {
            url += "?";
        }
        if (string::npos != url.find('&'))
        {
            url += "&";
        }
        url += kv;
    }
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    GLOG(IM_DEBUG, "set url=%s", url.c_str());
}

curl_slist* HttpClientMock::SetHttpHead(RRMessage* req)
{
    curl_slist* ch = NULL;
    EList<pair<string, string> >& vh = req->GetHttpHead();
    for (pair<string, string>* it = vh.begin();
            ! vh.end(); it = vh.next())
    {
        string s = it->first + ":" + it->second;
        ch = curl_slist_append(ch, s.c_str());
        GLOG(IM_INFO, "[send] add head [ %s ]", s.c_str());
    }
    //if (ch)
    {// 如果没有,也需要置空,否则会coredump
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, ch);
    }
    return ch;
}

int HttpClientMock::Send(QA* qa, RRMessage* req)
{
    int retcode = 0;
    if (! m_curl)
    {
        m_curl = curl_easy_init();
        if (NULL == m_curl)
        {
            return -1;
        }
    }
    SetPostFlag(req);
    SetUrl(req);
    curl_slist* ch = SetHttpHead(req);

    //设置其他属性
    *(int*)m_response = 0;
    *(int*)m_responseHead = 0;
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT_MS, m_timeout);
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, WriteData);
    curl_easy_setopt(m_curl, CURLOPT_WRITEHEADER, m_responseHead);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, m_response);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    GLOG(IM_INFO, "[send] query len=%u", req->GetBody().size());
    GLOG(IM_INFO, "[send] query body[0,1024]=%.*s", 
            1024, Str::ToPrint(req->GetBody()).c_str());

    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK)
    {
        GLOG(IM_ERROR, "curl_easy_perform() failed: %s",
                curl_easy_strerror(res));
        retcode = -3;
    }
    else
    {
        string head, code, body;
        string total;
        {
            long c;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &c);
            char buf[16];
            snprintf(buf, sizeof(buf), "%ld", c);
            code = buf;
        }
        {
            int size = *(int*)m_responseHead;
            char* p = m_responseHead + sizeof(int);
            head = string(p, size);
            total += head;
            head = head.substr(head.find("\r\n") + 2);
        }
        {
            int size = *(int*)m_response;
            char* p = m_response + sizeof(int);
            body = string(p, size);
            total += body;
        }
        GLOG(IM_INFO, "[recv] ------head------\n%s", head.c_str());
        GLOG(IM_INFO, "[recv] answer len=%d", body.size());
        GLOG(IM_INFO, "[recv] answer body[0,1024]=%.*s", 
                1024, Str::ToPrint(body).c_str());
        WriteMsg(total);
        RRMessage res;
        res.SetHeader(head);
        res.SetHttpCode(code);
        res.SetBody(body);
        if (! qa->Match(&res, false, ""))
        {
            GLOG(IM_DEBUG, "parse failed.");
            retcode = -2;
            goto END;
        }
        goto END;
    }
END:
    if (ch)
    {
        curl_slist_free_all(ch);
    }

    return retcode;
}
