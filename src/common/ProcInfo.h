/**
 * 文件名称：ProcInfo.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2016.11.24
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */

 #ifndef _PROCINFO_H
 #define _PROCINFO_H

#include <string>

 class ProcInfo
 {
     public:
        ProcInfo();
        void AddOneIn(long bytes);
        void AddOneOut(long bytes);
        void AddTime(double timeMs);
        std::string DebugString();
        static std::string DebugString(ProcInfo* pi, int size);
        ProcInfo& operator +=(const ProcInfo& pi);
        ProcInfo operator -(const ProcInfo& pi);
     public:
        long GetRequestNumIn();
        long GetRequestNumInBytes();
        long GetRequestNumOut();
        long GetRequestNumOutBytes();
        double GetRequestTimeMS();
     private:
        long m_requestNumIn;
        long m_requestNumInBytes;
        long m_requestNumOut;
        long m_requestNumOutBytes;
        double m_requestTimeMS;
 };


 #endif
