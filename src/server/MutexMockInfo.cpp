#include "MutexMockInfo.h"

using namespace std;

bool MutexMockInfo::Init(int threadNum, 
        const string& fmtFile, 
        const string& datFile,
        bool cachedDat,
        BaseMock* mockObj)
{
    if (m_datFmtObjs.empty())
    {
        pthread_mutex_init(&m_mutex,NULL);
        for (int i = 0; i < threadNum; ++i)
        {
            DatFmt* df = new DatFmt;
            df->dat = new Dat();
            df->fmt = new Fmt();
            df->fmt->Load(fmtFile.c_str());
            df->dat->Load(datFile.c_str(), df->fmt, "", "");
            m_datFmtObjs.push_back(df);
        }
        m_cachedDat = cachedDat;
        m_datFile = datFile;
        m_mockObj = mockObj;
    }
    return true;
}

DatFmt* MutexMockInfo::GetDatFmt()
{
    pthread_mutex_lock(&m_mutex);
    DatFmt* df = m_datFmtObjs.back();
    m_datFmtObjs.pop_back();
    pthread_mutex_unlock(&m_mutex);
    if (! m_cachedDat)
    {
        delete df->dat;
        df->dat = new Dat();
        if (! df->dat->Load(m_datFile.c_str(), df->fmt, "", ""))
        {
            return NULL;
        }
    }
    return df;
}

void MutexMockInfo::FreeDatFmt(DatFmt* df)
{
    pthread_mutex_lock(&m_mutex);
    m_datFmtObjs.push_back(df);
    pthread_mutex_unlock(&m_mutex);
}


void MutexMockInfo::WriteMsg(const string& msg)
{
    pthread_mutex_lock(&m_mutex);
    m_mockObj->WriteMsg(msg);
    pthread_mutex_unlock(&m_mutex);
}
