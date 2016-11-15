#include "Log.h"

using namespace log4cpp;
using namespace std;

Logger::Logger()
{
    m_file = "";
    m_maxBackupIndex = 1;
    m_level = IM_DEBUG;
    m_maxFileSize = 10 * 1024 * 1024;
    m_additivity = true;
    m_append = true;
    m_category = NULL;
    m_layout = NULL;
    m_appender = NULL;
    m_dev = NULL;
}

Logger::Logger(const std::string& file)
{
    m_file = file;
    m_maxBackupIndex = 1;
    m_level = IM_DEBUG;
    m_maxFileSize = 10 * 1024 * 1024;
    m_additivity = true;
    m_append = true;
    m_category = NULL;
    m_layout = NULL;
    m_appender = NULL;
    m_dev = NULL;
}

Logger::~Logger()
{
    //由于log4cpp版本里存在局部static变量
    //这里不能释放内存,否则会引起core
    if (m_layout)
    {
        //delete m_layout;
    }
    if (m_appender)
    {
        //delete m_appender;
    }
}

bool Logger::Load()
{
    errno = 0;
    if ("/dev/stdout" == m_file)
    {
        m_dev = stdout;
        return true;
    }
    else if ("/dev/stderr" == m_file)
    {
        m_dev = stderr;
        return true;
    }

    if (m_layout)
    {
        delete m_layout;
    }
    m_layout = new PatternLayout();
    m_layout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S} [%5p]: %m%n");
    //构造appender
    if (m_appender)
    {
        delete m_appender;
    }
    m_appender = new RollingFileAppender(
            "", m_file, m_maxFileSize, m_maxBackupIndex, m_append);
    m_appender->setLayout(m_layout);

    m_category = &(Category::getInstance(m_file));
    m_category->addAppender(m_appender);
    m_category->setPriority(m_level);
    m_category->setAdditivity(m_additivity);
    if (errno)
    {
        fprintf(stderr, "load %s failed, err:%s.\n", 
                m_file.c_str(), strerror(errno));
        return false;
    }
    return true;
}

void Logger::Write(log4cpp::Priority::Value pl, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (m_dev)
    {
        fprintf(m_dev, "[%5s]", Priority::getPriorityName(pl).c_str());
        vfprintf(m_dev, fmt, ap);
        fprintf(m_dev, "\n");
    }
    else if (m_category)
    {
        m_category->logva(pl, fmt, ap);
    }
    va_end(ap);
    return ;
}

/*--初始化日志部分--*/
static map<string, Priority::Value> Str2LogLevelMap()
{
    map<string, Priority::Value> mp;
    mp["emerg" ] = IM_EMERG;
    mp["fatal" ] = IM_FATAL;
    mp["alert" ] = IM_ALERT;
    mp["crit"  ] = IM_CRIT;
    mp["error" ] = IM_ERROR;
    mp["warn"  ] = IM_WARN;
    mp["notice"] = IM_NOTICE;
    mp["info"  ] = IM_INFO;
    mp["debug" ] = IM_DEBUG;
    return mp;
}


bool LoadLog(const string& val, void*data, uint64_t o)
{
    static map<string, Priority::Value> str2logLevel = Str2LogLevelMap();
    if (val.empty())
    {   
        return false;
    }       
    string file = "";
    string level = "";
    stringstream ss(val);

    ss >> file >> level;
    if (level.empty())
    {
        level = "info";
    }
    delete g_log;
    g_log = new Logger;
    g_log->SetFile(file);
    map<string, Priority::Value>::const_iterator it; 
    it = str2logLevel.find(level);
    if (str2logLevel.end() == it)
    {
        printf("unrecognized log level:%s", level.c_str());
        return false;
    }
    g_log->SetLevel(it->second);
    return g_log->Load();
}
