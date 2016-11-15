/**
 * 文件名称：imock-interface.h
 * 摘要：对外的编程接口，可以通过该接口对字符串序列化、反序列化、匹配等操作
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.03.05
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */

#ifndef _IMOCK_INTERFACE_H_
#define _IMOCK_INTERFACE_H_

#include <string>
#include <vector>

typedef struct 
{
    std::vector<std::string> q;
    std::string qfid;
    std::vector<std::string> a;
    std::string afid;
}IQA;

class ImockInterface
{
    public:
        ImockInterface();
        ~ImockInterface();
        // 设置日志级别，支持debug, info, error三种级别，
        // 不设置默认是info
        // 需要在一开始就调用，否则不生效
        bool Init(const std::string& logLevel);

        // 加载fmt文件
        bool LoadFmtFile(const std::string& fmtFile);

        // 可选，加载dat文件
        bool LoadDatFile(const std::string& datFile, const std::string& qfid, const std::string& afid);

        // 解析src，将结果存储在xml中
        // isq 是否用q节点解析，false则用a节点解析
        // fid 用来指定使用哪个fid标记进行解析,如果不指定,则会顺序尝试
        // pobj 用来存储内部的节点数据, 可以用于后续的Match调用,需要在外面显示调用Delete
        bool Parse(const std::string& src, 
                std::string& xml, 
                bool isq = true, 
                const std::string& fid = "", void** pobj = NULL,
                const std::string& sepb = "",
                const std::string& sepe = "");

        // 解析并匹配
        // 同Parse函数,在解析完成后会对其dat.xml里的节点进行匹配,并将匹配的节点以cobj返回,不需要释放该节点
        bool PMatch(const std::string& src, std::string& xml, bool isq = true, const std::string& fid= "", void** pobj = NULL, void** cobj = NULL);

        // 序列化
        bool Serialize(std::vector<IQA>& dst);

        // 匹配
        // 将匹配pobj1是否和cobj2里的节点匹配,
        bool Match(void* pobj1, void* cobj2, bool isq = true, const std::string& fid = "");

        // 对于Parse返回的pobj, 需要调用Delete
        void Delete(void* pobj);

    private:
        bool Serialize(void* r, std::vector<std::string>& v, const std::string& fid);

    private:
        void* m_fmt;
        void* m_dat;
};

#endif
