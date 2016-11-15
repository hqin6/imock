/**
 * 文件名称：QA.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.02
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _QA_H_
#define _QA_H_

#include <vector>
#include "RRMessage.h"
#include "DatNodeRoot.h"
#include "Alloc.h"

class QA : public Alloc
{
    public:
        QA();
        ~QA();
        bool Load(TiXmlElement* e, Fmt* fmt, 
                const std::string& qfid,
                const std::string& afid);
        //是否匹配成功
        bool Match(RRMessage* msg, bool isQ, const std::string& fid);
        //返回其中的一个answer
        bool Query(RRMessage* dst, const std::string& qID = "");
        bool Answer(RRMessage* dst, RRMessage* src = NULL);
        std::vector<DatNodeRoot*>& GetQ();
        std::vector<DatNodeRoot*>& GetA();
    public:
        const char* GetDebugID();
    private:
        DatNodeRoot* GetByRate(std::vector<DatNodeRoot*>& vec, const std::string& qID = "");
    private:
        std::vector<DatNodeRoot*> m_vecQ;
        std::vector<DatNodeRoot*> m_vecA;
        const char* m_debugId;
};


#endif
