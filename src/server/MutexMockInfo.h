/**
 * 文件名称：MutexMockInfo.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.04.03
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _MUTEXMOCKINFO_H_
#define _MUTEXMOCKINFO_H_

#include <list>
#include "Fmt.h"
#include "Dat.h"
#include "BaseMock.h"

typedef struct
{
    Dat* dat;
    Fmt* fmt;
}DatFmt;

class MutexMockInfo
{
    public:
        bool Init(int threadNum, const std::string& fmtFile,
                const std::string& datFile, bool cachedDat, BaseMock* mockObj);
        DatFmt* GetDatFmt();
        void FreeDatFmt(DatFmt* df);
        void WriteMsg(const std::string& msg);

    private:
        bool m_cachedDat;
        std::string m_datFile;
        std::list<DatFmt*> m_datFmtObjs;
        BaseMock* m_mockObj;
        pthread_mutex_t m_mutex;
};


#endif
