/**
 * 文件名称：RRMessage.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.10.25
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _RRMESSAGE_H_
#define _RRMESSAGE_H_

#include <string>
#include <set>
#include <map>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
#include "EList.h"
#include "Alloc.h"

class DatNode;
class FmtNode;

class RRMessage : public Alloc
{
    public:
        RRMessage();
        virtual ~RRMessage();
        /*
         * 解析相关
         */
        void SetCurFid(const std::string& fid);
        //是否成功解析过
        bool Parsed();
        //检查是否是黑名单
        bool BlackFid(const std::string& fid);
        //增加黑名单
        void AddBlackFid(const std::string& fid);
        /*
         * 变量相关
         */
        //获取变量
        const char* GetVar(const std::string& v, bool& found);
        //增加变量
        void AddVar(const std::string& k, const std::string& v);
        void DelVar(const std::string& k);
        //清除变量
        void ClearVars();
        /*
         * 成员设置和获取接口
         */
        EList<DatNode*>& GetNode();
        void AddNode(DatNode* root);

        void SetFmtNode(FmtNode* n);
        FmtNode* GetFmtNode();

        void SetMethod(const std::string& s);
        const std::string& GetMethod();

        void SetService(const std::string& s);
        const std::string& GetService();

        const std::string& GetUriPath();
        void SetUriPath(const std::string& s);

        void SetUriQuery(const std::string& s);
        const std::string& GetUriQuery();

        void AddHttpUriQuery(const std::string& k, const std::string& v);
        EList<std::pair<std::string, std::string> >& GetHttpUriQuery();

        const std::string& GetHeader();
        void SetHeader(const std::string& s);

        void SetBody(const std::string& body);
        const std::string& GetBody();

        void SetUSleep(int usleep);
        int GetUSleep();

        void SetHttpCode(const std::string& c);
        const std::string& GetHttpCode();

        void SetHttpCodeMsg(const std::string& msg);
        const std::string& GetHttpCodeMsg();

        void SetHttpVersionMinor(int m);
        int GetHttpVersionMinor();

        void SetFailedMsg(const std::string& msg);
        const std::string& GetFailedMsg();
        bool HasFailedMsg();

        EList<std::pair<std::string, std::string> >& GetHttpHead();
        void AddHttpHead(const std::string& k, const std::string& v);

        /*
         * 其他
         */
        std::string DebugString();
        std::string DebugXmlString(const std::string& sepb = "",
                const std::string& sepe = "");

    private:
        //对于某些fid，解析失败了，就不需要再解析了
        std::set<std::string> m_blackFid;
        std::map<std::string, EList<DatNode*>* > m_nodes;
        // 某一个fid对应的fmt
        std::map<std::string, FmtNode*> m_fmtNodes;
        //变量名,变量值
        std::map<std::string, std::map<std::string, std::string> > m_vars;
        std::string m_curFid;
        std::string m_body;

        EList<std::pair<std::string, std::string> > m_httpHead;
        std::string m_header;

        EList<std::pair<std::string, std::string> > m_httpUriQuery;
        std::string m_uriQuery;

        std::string m_httpCodeMsg;
        std::string m_method;
        std::string m_service;
        std::string m_uriPath;
        std::string m_httpCode;
        std::string m_failedMsg;
        int m_usleep;
        int m_httpVersionMinor;
        bool m_hasFailedMsg;
};


#endif
