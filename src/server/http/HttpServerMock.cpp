#include "HttpServerMock.h"
#include "Log.h"
#include "INIConf.h"
#include "event2/buffer.h"
#include "event2/keyvalq_struct.h"
#include "event2/http_struct.h"
#include "Time.h"
#include "EList.h"
#include "Str.h"
#include <list>
#include <openssl/ssl.h>
#include <event2/bufferevent_ssl.h>

using namespace std;

static map<int, string> HttpCodeMsg()
{
    map<int, string> mp;
    mp[100]="Continue";
    mp[101]="Switching Protocols";
    mp[200]="OK";
    mp[201]="Created";
    mp[202]="Accepted";
    mp[203]="Non-Authoritative Information (for DNS)";
    mp[204]="No Content";
    mp[205]="Reset Content";
    mp[206]="Partial Content";
    mp[300]="Multiple Choices";
    mp[301]="Moved Permanently";
    mp[302]="Moved Temporarily";
    mp[303]="See Other";
    mp[304]="Not Modified";
    mp[305]="Use Proxy";
    mp[307]="Redirect Keep Verb";
    mp[400]="Bad Request";
    mp[401]="Unauthorized";
    mp[402]="Payment Required";
    mp[403]="Forbidden";
    mp[404]="Not Found";
    mp[405]="Bad Request";
    mp[406]="Not Acceptable";
    mp[407]="Proxy Authentication Required";
    mp[408]="Request Timed-Out";
    mp[409]="Conflict";
    mp[410]="Gone";
    mp[411]="Length Required";
    mp[412]="Precondition Failed";
    mp[413]="Request Entity Too Large";
    mp[414]="Request, URI Too Large";
    mp[415]="Unsupported Media Type";
    mp[500]="Internal Server Error";
    mp[501]="Not Implemented";
    mp[502]="Bad Gateway";
    mp[503]="Server Unavailable";
    mp[504]="Gateway Timed-Out";
    mp[505]="HTTP Version not supported";
    return mp;
}

static const string GetHttpCodeMsg(int code)
{
    static map<int, string> mp = HttpCodeMsg();
    map<int, string>::iterator it;
    it = mp.find(code);
    if (it == mp.end())
    {
        return "";
    }
    return it->second;
}

#ifdef HTTPS_SUPPORT
/**
 *  * This callback is responsible for creating a new SSL connection
 *   * and wrapping it in an OpenSSL bufferevent.  This is the way
 *    * we implement an https server instead of a plain old http server.
 *     */
static struct bufferevent* bevcb(struct event_base *base, void *arg)
{ struct bufferevent* r;
    SSL_CTX *ctx = (SSL_CTX *) arg;

    r = bufferevent_openssl_socket_new (base,
            -1,
            SSL_new (ctx),
            BUFFEREVENT_SSL_ACCEPTING,
            BEV_OPT_CLOSE_ON_FREE);
    return r;
} 

static void* ZeroingMalloc(size_t howmuch)
{ 
    return calloc (1, howmuch); 
}
#endif

HttpServerMock::HttpServerMock(const string& name, bool useSSL)
    : BaseServerMock(name), m_port(0), m_useSSL(useSSL)
{
}

HttpServerMock::~HttpServerMock()
{
}

bool HttpServerMock::Init()
{
    if (! BaseServerMock::Init())
    {
        return false;
    }
    Command c[] = {
        { m_name.c_str(), "port",         ISet::Int, (uint64_t)&m_port,       NULL},
        { m_name.c_str(), "ssl_cert",     ISet::Str, (uint64_t)&m_sslCert,    "" },
        { m_name.c_str(), "ssl_cert_key", ISet::Str, (uint64_t)&m_sslCertKey, ""},
        { NULL,           NULL,      NULL,      0           , NULL}
    };
    if (! g_iniConf->Get(c, 0))
    {   
        return false;
    }
    if (m_useSSL) 
    {
        #ifdef HTTPS_SUPPORT
        if (m_sslCert.empty() || m_sslCertKey.empty())
        {
            GLOG(IM_ERROR, "no ssl_cert or ssl_cert_key file"); 
            return false;
        }
        #else
        GLOG(IM_ERROR, "https not support, please recompile with -DHTTPS_SUPPORT"); 
        return false;
        #endif
    }
    return true;
}

int HttpServerMock::Loop()
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500*1000;
    if (0 != event_reinit(m_base))
    {
        return -1;
    }
    evtimer_add(m_exitEv, &timeout);
    event_base_dispatch(m_base);
    return 0;
}

int HttpServerMock::BeforeFork()
{
    if (0 != BaseServerMock::BeforeFork())
    {
        return -1;
    }
    if (m_useSSL)
    {
        #ifdef HTTPS_SUPPORT
        CRYPTO_set_mem_functions(ZeroingMalloc, realloc, free);
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        #else
        GLOG(IM_ERROR, "https not support, please recompile with -DHTTPS_SUPPORT"); 
        return -1;
        #endif
    }
    //new event base
    m_base = event_base_new();
    if (! m_base)
    {
        GLOG(IM_ERROR, "Couldn't create an event_base: exiting");
        return -1;
    }
    //注册超时回调函数
    m_tm = evtimer_new(m_base, HttpServerMock::TimerCallBack, this);
    m_exitEv = evtimer_new(m_base, HttpServerMock::InterruptLoop, this);
    if (! m_tm || ! m_exitEv)
    {
        GLOG(IM_ERROR, "Couldn't create evtimer. Exiting.");
        return -1;
    }
    m_http = evhttp_new(m_base);
    if (! m_http)
    {
        GLOG(IM_ERROR, "Couldn't create evhttp. Exiting.");
        return -1;
    }
    if (m_useSSL)
    {
        #ifdef HTTPS_SUPPORT
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
        SSL_CTX_set_options(ctx,
                SSL_OP_SINGLE_DH_USE |
                SSL_OP_SINGLE_ECDH_USE |
                SSL_OP_NO_SSLv2);
        EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
        if (! ecdh)
        {
            GLOG(IM_ERROR, "EC_KEY_new_by_curve_name");
            return -1;
        }
        if (1 != SSL_CTX_set_tmp_ecdh (ctx, ecdh))
        {
            GLOG(IM_ERROR, "SSL_CTX_set_tmp_ecdh");
            return -1;
        }
        GLOG(IM_DEBUG, "Loading certificate chain from %s, and private key from %s",
                m_sslCert.c_str(), m_sslCertKey.c_str());
        if (1 != SSL_CTX_use_certificate_chain_file(ctx, m_sslCert.c_str()))
        {
            GLOG(IM_ERROR, "SSL_CTX_use_certificate_chain_file: %s", m_sslCert.c_str());
            return -1;
        }
        if (1 != SSL_CTX_use_PrivateKey_file(ctx, m_sslCertKey.c_str(), SSL_FILETYPE_PEM))
        {
            GLOG(IM_ERROR, "SSL_CTX_use_PrivateKey_file: %s", m_sslCertKey.c_str());
            return -1;
        }
        if (1 != SSL_CTX_check_private_key(ctx))
        {
            GLOG(IM_ERROR, "SSL_CTX_check_private_key");
            return -1;
        }
        evhttp_set_bevcb(m_http, bevcb, ctx);
        #else
        GLOG(IM_ERROR, "https not support, please recompile with -DHTTPS_SUPPORT"); 
        return -1;
        #endif
    }

    evhttp_set_gencb(m_http, HttpServerMock::Process, this);
    m_handle = evhttp_bind_socket_with_handle(m_http, "0.0.0.0", m_port);
    if (! m_handle)
    {
        GLOG(IM_ERROR, "Couldn't bind to port %d. Exiting", m_port);
        return -1;
    }
    GLOG(IM_DEBUG, "bind on port %d ok.", m_port);
    return 0;
}

void HttpServerMock::InterruptLoop(int fd, short kind, void* userp)
{   
    HttpServerMock* hm = (HttpServerMock*)userp;
    if (s_stopFlag)
    {//需要退出
        event_base_loopexit(hm->m_base, NULL);
    }
    else
    {//下一次计时，500ms
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500*1000;
        evtimer_add(hm->m_exitEv, &timeout);
    } 
}

static map<struct timeval, list<struct TimerInfo*>,
        bool (*)(const struct timeval&, const struct timeval&) > s_ti(Time::LT);

void HttpServerMock::TimerCallBack(int fd, short kind, void* userp)
{
    //当前时间
    struct timeval tv;
    gettimeofday(&tv, NULL);
    map<struct timeval, list<struct TimerInfo*> >::iterator it;
    for (it = s_ti.begin(); it != s_ti.end(); ++it)
    {
        if (! Time::LT(it->first, tv))
        {
            break;
        }
        for (list<struct TimerInfo*>::iterator i = it->second.begin(); i != it->second.end(); i++)
        {
            Send((*i)->req, (*i)->res, (*i)->recvMoment, userp);
            delete *i;
        }
    }
    s_ti.erase(s_ti.begin(), it);
    return ;
}


void HttpServerMock::Process(struct evhttp_request *req, void *arg)
{
    const struct evhttp_uri* eu = evhttp_request_get_evhttp_uri(req);
    HttpServerMock* hm = (HttpServerMock*)arg;
    RRMessage r;
    struct timeval recvMoment;
    gettimeofday(&recvMoment, NULL);

    //http的请求信息（包括http 头部)
    string query = "";
    //获取http头信息的cmd
    const char *cmdtype;
    switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET:     cmdtype = "GET";     break;
        case EVHTTP_REQ_POST:    cmdtype = "POST";    break;
        case EVHTTP_REQ_HEAD:    cmdtype = "HEAD";    break;
        case EVHTTP_REQ_PUT:     cmdtype = "PUT";     break;
        case EVHTTP_REQ_DELETE:  cmdtype = "DELETE";  break;
        case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE:   cmdtype = "TRACE";   break;
        case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
        case EVHTTP_REQ_PATCH:   cmdtype = "PATCH";   break;
        default:                 cmdtype = "unknown"; break;
    }
    r.SetMethod(cmdtype);
    query += cmdtype;
    //获取http头信息的uri
    const char* uri = evhttp_uri_get_path(eu);
    if (uri)
    {
        r.SetUriPath(uri);
        query = query + " " + uri;
    }
    //获取http头信息的GET参数
    const char* args = evhttp_uri_get_query(eu);
    if (args)
    {
        r.SetUriQuery(args);
        query = query + "?" + args;
    }
    //获取HTTP版本信息
    {
        char buf[32];
        snprintf(buf, sizeof(buf), " HTTP/%d.%d\r\n", 
                req->major, req->minor);
        query += buf;
    }
    //获取http头信息的kv头部
    struct evkeyvalq *headers;
    headers = evhttp_request_get_input_headers(req);
    string h = "";
    for (struct evkeyval* header = headers->tqh_first; header;
            header = header->next.tqe_next)
    {
        string k(header->key);
        string v(header->value);
        h += k + ":" + v + "\r\n";
    }
    r.SetHeader(h);
    query += h;
    //获取POST的body
    string body;
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    while (evbuffer_get_length(buf))
    {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(cbuf)-1);
        if (n > 0)
        {
            body += string(cbuf, n);
        }
    }

    r.SetBody(body);
    GLOG(IM_INFO, "[recv] ------head------\n%s", h.c_str());
    GLOG(IM_INFO, "[recv] query len=%d", body.size());
    GLOG(IM_INFO, "[recv] query body[0,1024]=%.*s", 
            1024, Str::ToPrint(body).c_str());
    query += "\r\n" + body + "\r\n";
    if (hm->m_mmapProcInfo)
    {
        hm->m_mmapProcInfo->AddOneIn(query.length());
        GLOG(IM_DEBUG, "realinfo: add one in: %ld", query.length());
    }
    //记录query
    hm->WriteMsg(query);

    //拼接返回的数据，并返回
    if (! hm->m_cachedDat)
    {
        delete hm->m_dat;
        hm->m_dat = new Dat();
        if (! hm->m_dat->Load(hm->m_datFile.c_str(), hm->m_fmt, "", ""))
        {
            GLOG(IM_ERROR, "load dat file failed, send nothing to client.");
            return ;
        }
        GLOG(IM_DEBUG, "reload dat file ok.");
    }
    QA* qa = hm->m_dat->Match(&r, true, "");
    if (! qa)
    {
        GLOG(IM_ERROR, "no match dat, send nothing to client.");
        return ;
    }
    RRMessage* resp = new RRMessage;
    bool res = qa->Answer(resp, &r);
    if (! res)
    {
        GLOG(IM_ERROR, "no answer, send nothing to client.");
        delete resp;
        return ;
    }
    int us = resp->GetUSleep();
    if (us <= 0)
    {//立即发送
        Send(req, resp, recvMoment, hm);
        delete resp;
        return ;
    }
    //将其加入等待队列，增加超时事件
    TimerInfo* ti = new TimerInfo;
    ti->req = req;
    ti->res = resp;
    ti->recvMoment = recvMoment;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Time::AddUS(tv, us);
    s_ti[tv].push_back(ti);
    GLOG(IM_DEBUG, "will send %p after usleep %d.", ti, us);
    //增加4ms的时间精度窗口
    struct timeval timeout;
    timeout.tv_sec = (us + 4000) / 1000000;
    timeout.tv_usec = (us + 4000) % 1000000;
    evtimer_add(hm->m_tm, &timeout);
}

void HttpServerMock::Send(struct evhttp_request* req, 
        RRMessage* res, 
        struct timeval& recvMoment,
        void* arg)
{
    struct evbuffer *evb = evbuffer_new();
    EList<pair<string, string> >& v = res->GetHttpHead();
    pair<string, string>* it;
    for (it = v.begin(); ! v.end(); it = v.next())
    {
        evhttp_add_header(evhttp_request_get_output_headers(req),
                it->first.c_str(), it->second.c_str());
        GLOG(IM_INFO, "[send] add head [ %s:%s ]", 
                it->first.c_str(),
                it->second.c_str());
    }

    evbuffer_add(evb, res->GetBody().c_str(), res->GetBody().size());
    GLOG(IM_DEBUG, "send ok, len=%d", res->GetBody().size());
    string codeStr = res->GetHttpCode();
    int code = 200;
    string msg = res->GetHttpCodeMsg();
    if (! codeStr.empty())
    {
        code = atoi(codeStr.c_str());
    }
    if (msg.empty())
    {
        msg = GetHttpCodeMsg(code);
        if (msg.empty())
        {
            code = 200;
            msg = "OK";
        }
    }
    int m = res->GetHttpVersionMinor();
    if (m == 0 || m == 1)
    {// 只支持0/1
        req->minor = m;
    }

    evhttp_send_reply(req, code, msg.c_str(), evb);
    if (((HttpServerMock*)arg)->m_mmapProcInfo)
    {
        ((HttpServerMock*)arg)->m_mmapProcInfo->AddOneOut(
            evbuffer_get_length(bufferevent_get_output(evhttp_connection_get_bufferevent(req->evcon))));
        GLOG(IM_DEBUG, "realinfo: add one out: %ld", 
                evbuffer_get_length(bufferevent_get_output(evhttp_connection_get_bufferevent(req->evcon))));
        struct timeval tmp;
        gettimeofday(&tmp, NULL);
        double spentMs = Time::SubTimeMs(tmp, recvMoment);
        ((HttpServerMock*)arg)->m_mmapProcInfo->AddTime(spentMs);
        GLOG(IM_DEBUG, "realinfo: add spent(ms): %lf", spentMs);
    }
    GLOG(IM_INFO, "[send] answer len=%d", res->GetBody().size());
    GLOG(IM_INFO, "[send] answer body[0,1024]=%.*s", 
            1024, Str::ToPrint(res->GetBody()).c_str());
    evbuffer_free(evb);
    return ;
}
