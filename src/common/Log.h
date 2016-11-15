/**
 * 文件名称：Log.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2013.12.04
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include "log4cpp/Category.hh"
#include "log4cpp/RollingFileAppender.hh"
#include "log4cpp/PatternLayout.hh"

#define LOG(_logger, level, fmt, ...) \
    do { \
        if (_logger && (_logger)->GetLevel() >= level) \
        { \
            (_logger)->Write(level, "[%d] " fmt, getpid(), ##__VA_ARGS__);\
        } \
    }while(0)

#define GLOG(level, fmt, ...) \
    LOG(g_log, level, fmt, ##__VA_ARGS__)

#define IM_EMERG  ::log4cpp::Priority::EMERG
#define IM_FATAL  ::log4cpp::Priority::FATAL
#define IM_ALERT  ::log4cpp::Priority::ALERT
#define IM_CRIT   ::log4cpp::Priority::CRIT
#define IM_ERROR  ::log4cpp::Priority::ERROR
#define IM_WARN   ::log4cpp::Priority::WARN
#define IM_NOTICE ::log4cpp::Priority::NOTICE
#define IM_INFO   ::log4cpp::Priority::INFO
#define IM_DEBUG  ::log4cpp::Priority::DEBUG

class Logger
{
    public:
        Logger();
        Logger(const std::string& file);
        ~Logger();
        bool Load();
        void Write(log4cpp::Priority::Value pl,
                const char* fmt, ...);

    public:
        void SetFile(const std::string& file)
        {
            m_file = file;
        }
        void SetMaxBackupIndex(unsigned int idx)
        {
            m_maxBackupIndex = idx;
        }
        void SetLevel(log4cpp::Priority::Value pl)
        {
            m_level = pl;
        }
        void SetMaxFileSize(size_t size)
        {
            m_maxFileSize = size;
        }
        void SetAdditivity(bool additivity)
        {
            m_additivity = additivity;
        }
        void SetAppend(bool append)
        {
            m_append = append;
        }
        log4cpp::Priority::Value GetLevel()
        {
            return m_level;
        }
        FILE* GetDev()
        {
            return m_dev;
        }

    private:
        std::string m_file;               //日志文件
        unsigned int m_maxBackupIndex;    //
        log4cpp::Priority::Value m_level; //日志级别
        size_t m_maxFileSize;             //日志轮转大小
        bool m_additivity;                //是否需要附加信息
        bool m_append;
        FILE* m_dev;                      //stdout/stderr

    private:
        log4cpp::Category*  m_category;    //日志载体
        log4cpp::PatternLayout* m_layout;
        log4cpp::Appender*  m_appender;
};

extern Logger* g_log;

bool LoadLog(const std::string& val, void*data, uint64_t o);

#endif
