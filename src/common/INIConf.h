/**
 * 文件名称：INIConf.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2013.12.04
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _INICONF_H_
#define _INICONF_H_

#include <string>
#include <map>
#include <stdint.h>

/*适用于指令的回调函数*/
typedef bool (*Set)(const std::string& s, void* d, uint64_t o);

/*配置相关信息,适用于多个指令的回调函数*/
struct Command
{
    const char* sec; //配置项存在的sec
    const char* key; //配置项的key
    Set         set;
    uint64_t offset; //配置项的值在conf里的偏移
    const char* def; //默认值
};

/*常用的set接口*/
class ISet
{
    public:
        //支持int读取
        static bool Int(const std::string& s, void* d, uint64_t o);
        //支持string读取
        static bool Str(const std::string& s, void* d, uint64_t o);
        //支持on/off读取
        static bool Flag(const std::string& s, void* d, uint64_t o);
        //支持100K读取
        static bool Size(const std::string& s, void* d, uint64_t o);
};


class INIConf
{
    //存储key = value
    typedef std::map<std::string, std::string> KVMap;
    //存储 [sec] 里的所有key = value
    typedef std::map<std::string, KVMap> SecMap;

    public:
        /* 加载配置文件ini, 返回成功或者失败*/
        bool Load(const std::string& ini);
        /*适用于单个指令的获取*/
        bool Get(const std::string& sec, 
                const std::string& key, Set set, void* data, const char* def = NULL);
        /*适用于多个指令（一个结构体）的获取*/
        bool Get(const Command* c, void* data);
        /* 只是用于测试*/
        std::string Dump() const;
    private:
        bool Get(const std::string& sec,
                const std::string& key,
                std::string& val);
    private:
        SecMap m_secMap;
};

//全局的配置
extern INIConf* g_iniConf;

#endif
