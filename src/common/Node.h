/**
 * 文件名称：Node.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.13
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _NODE_H_
#define _NODE_H_

#include "EList.h"
#include "tinyxml.h"
#include "Alloc.h"

#define OP_URL_ENC                   0x00000001
#define OP_URL_DEC                   0x00000002
#define OP_GZIP                      0x00000004
#define OP_GUNZIP                    0x00000008
#define OP_ZIP                       0x00000010
#define OP_UNZIP                     0x00000020
#define OP_RVAR                      0x00000040
#define OP_RTRIM                     0x00000080
#define OP_LTRIM                     0x00000100
#define OP_TRIM                      0x00000200
#define OP_SHELL                     0x00000400
#define OP_SHELL_HEAD                0x00000800
#define OP_SHELL_BODY                0x00001000
#define OP_SHELL_RVALUE              0x00002000
#define OP_SHELL_RVAR                0x00004000
#define OP_CACHED                    0x00008000
#define OP_ESC                       0x00010000
#define OP_B64ENC                    0x00020000
#define OP_B64ENC_SAFE               0x00040000
#define OP_B64ENC_SAFE_NOAPPEND      0x00080000
#define OP_B64ENC_NOAPPEND           0x00100000
#define OP_B64DEC                    0x00200000
#define OP_B64DEC_SAFE               0x00400000
#define OP_B64DEC_SAFE_NOAPPEND      0x00800000
#define OP_B64DEC_NOAPPEND           0x01000000
#define OP_SHELL_NO_IN_FILE          0x02000000
#define OP_NO_AUTO_CVT               0x04000000
// end for 0x80000000

#include "RRMessage.h"
class Node;
class OPHandler
{
    typedef std::string (*_s_cs)(const std::string&);
    typedef void (*_v_s_cs)(std::string&, const std::string&);
    typedef std::string (*_s_s_r)(std::string&, RRMessage*);
    typedef std::string (*_s_s_r_n_cs)(std::string&, RRMessage*, Node*, const std::string&);
    public:
    OPHandler(_s_cs h);
    OPHandler(_v_s_cs h);
    OPHandler(_s_s_r h);
    OPHandler(_s_s_r_n_cs h);
    void Exec(std::string& v, RRMessage* m, Node* n, const std::string& rv);
    private:
    union {
        _s_cs s_cs;
        _v_s_cs v_s_cs;
        _s_s_r s_s_r;
        _s_s_r_n_cs s_s_r_n_cs;
    }handler;
    enum {
        S_CS,
        V_S_CS,
        S_S_R,
        S_S_R_N_CS
    }type;
};

class Node : public Alloc
{
    typedef Node*(*NodeFactory)(TiXmlElement*);
    public:
        Node();
        virtual ~Node();
        //清空数据
        virtual void Clear();

        //从xml中load数据
        virtual bool Load(TiXmlElement* e, Node* parent = NULL);
        virtual bool LoadOP(const std::string& op, TiXmlElement* e);
        virtual std::string ExecOP(std::string& v, RRMessage* msg, const std::string& rv);

        static void SetFactory(NodeFactory nf);

        void AddChild(Node* c);
        void SetName(const std::string& name);
        const std::string& GetName();
        const std::string GetFullName();
        EList<Node*>& GetSubs();
        Node* GetSub(const std::string& name);
        long GetOP();

    protected:
        virtual  bool LoadAttr(TiXmlElement* e);
        static void ShellArgs(EList<std::string>& cmdArr, bool replaceVar, RRMessage* r, std::vector<std::string>& res);
    private:
        static pthread_mutex_t* NewFactoryMutex();
        static std::string ReplaceVar(std::string& v, RRMessage* msg);
        static std::string Shell(std::string& v, RRMessage* r, Node* n, const std::string& rv);

    public:
        virtual void SetAttr(Node* n);

    protected:
        //节点名称
        std::string m_name;
        //子节点
        EList<Node*> m_subs;
        //父节点
        Node* m_parent;
        //操作链
        std::vector<OPHandler> m_opFun;
        long m_op;
        const char* m_shellCmd;
        EList<std::string> m_shellCmdArr;
    private:
        static NodeFactory s_nf;
        static pthread_mutex_t* s_factoryMutex;
        static std::map<std::string, std::string> s_shellCache;
};


#endif
