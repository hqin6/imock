#include "ProcInfo.h"

using namespace std;

ProcInfo::ProcInfo()
{
    m_requestNumIn = 0;
    m_requestNumInBytes = 0;
    m_requestNumOut = 0;
    m_requestNumOutBytes = 0;
    m_requestTimeMS = 0;
}

void ProcInfo::AddOneIn(long bytes)
{
    m_requestNumIn++;
    m_requestNumInBytes += bytes;
}

void ProcInfo::AddOneOut(long bytes)
{
    m_requestNumOut++;
    m_requestNumOutBytes += bytes;
}

void ProcInfo::AddTime(double timeMs)
{
    m_requestTimeMS += timeMs; 
}

string ProcInfo::DebugString()
{
    char buf[128];
    snprintf(buf, sizeof(buf), "in:%ld(%ld bytes),out:%ld(%ld bytes),time:%lf", 
            m_requestNumIn, m_requestNumInBytes,
            m_requestNumOut, m_requestNumOutBytes, 
            m_requestTimeMS);
    return buf;
}

ProcInfo& ProcInfo::operator +=(const ProcInfo& pi)
{
    m_requestNumIn       += pi.m_requestNumIn;
    m_requestNumInBytes  += pi.m_requestNumInBytes;
    m_requestNumOut      += pi.m_requestNumOut;
    m_requestNumOutBytes += pi.m_requestNumOutBytes;
    m_requestTimeMS      += pi.m_requestTimeMS;
    return *this;
}

string ProcInfo::DebugString(ProcInfo* pi, int size)
{
    ProcInfo tmp;
    for (int i = 0; i < size; ++i)
    {
        tmp += *(pi + i);
    }
    return tmp.DebugString();
}

ProcInfo ProcInfo::operator -(const ProcInfo& pi)
{
    ProcInfo tmp;
    tmp.m_requestNumIn       = m_requestNumIn       - pi.m_requestNumIn;
    tmp.m_requestNumInBytes  = m_requestNumInBytes  - pi.m_requestNumInBytes;
    tmp.m_requestNumOut      = m_requestNumOut      - pi.m_requestNumOut;
    tmp.m_requestNumOutBytes = m_requestNumOutBytes - pi.m_requestNumOutBytes;
    tmp.m_requestTimeMS      = m_requestTimeMS      - pi.m_requestTimeMS;
    return tmp;
}

long ProcInfo::GetRequestNumIn()
{
    return m_requestNumIn;
}
long ProcInfo::GetRequestNumInBytes()
{
    return m_requestNumInBytes;
}
long ProcInfo::GetRequestNumOut()
{
    return m_requestNumOut;
}
long ProcInfo::GetRequestNumOutBytes()
{
    return m_requestNumOutBytes;
}
double ProcInfo::GetRequestTimeMS()
{
    return m_requestTimeMS;
}
