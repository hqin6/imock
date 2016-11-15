/**
 * 文件名称：Dat.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.02
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DAT_H_
#define _DAT_H_

#include <string>
#include "EList.h"
#include "RRMessage.h"
#include "QA.h"
#include "Alloc.h"

class Dat : public Alloc
{
    public:
        Dat();
        ~Dat();
        bool Load(const char* datFile, Fmt* fmt, 
                const std::string& qfid, const std::string& afid);
        //在dat.xml中寻找匹配于msg的数据，并返回需要返回的数据
        //msg里包含所有请求信息，比如http信息，
        //这些信息只有在fmt.xml里配置了才会读取
        QA* Match(RRMessage* msg, bool isQ, const std::string& fid);
        EList<QA*>& GetQA();
    private:
        EList<QA*> m_vecQA;
        TiXmlDocument* m_doc;
};


#endif
