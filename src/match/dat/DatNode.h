/**
 * 文件名称：DatNode.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.01
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DATNODE_H_
#define _DATNODE_H_

#define MATCH_REG           0x0001
#define MATCH_ICASE         0x0002
#define MATCH_LT            0x0004
#define MATCH_EQ            0x0008
#define MATCH_NE            0x0010
#define MATCH_GT            0x0020
#define MATCH_FMT_ERR       0x0040
#define MATCH_INT           0x0080
#define MATCH_NO_EXIST      0x0100
#define MATCH_FMT_OK        0x0200
#define MATCH_EXIST         0x0400
#define MATCH_SHELL         0x0800
#define MATCH_SHELL_RVAR    0x1000

/* in base node
 * 0x .... .... .... .... xxxx xxxx xxxx xxxx
*/
#define OP_HIDE    0x0000000100000000
#define OP_NOPACK  0x0000000200000000

#define FROM_FILE           0x0001
#define FILE_CACHED         0x0002
#define FROM_VAR            0x0004
//#define FROM_SHELL          0x0008
//#define FROM_SHELL_HEAD     0x0010
//#define FROM_SHELL_BODY     0x0020
//#define FROM_SHELL_RVALUE   0x0040
//#define FROM_SHELL_RVAR     0x0080

#include <string>
#include <map>
#include "Node.h"
#include "Reg.h"
#include "RRMessage.h"

enum MATCH_RESULT
{
    UNMATCH_NAME,
    UNMATCH_VALUE,
    UNMATCH_FMT,
    INNER_ERROR,
    MATCH_OK
};

class DatNode : public Node
{
    public:
        DatNode();
        ~DatNode();
        //获取字段值，可能从file里获取
        const std::string GetValue(RRMessage* msg = NULL, const std::string& rv = "");
        //设置value值
        void SetValue(const std::string& v);
        bool LoadAttr(TiXmlElement* e);
        std::string DebugString(int b);
        std::string DebugXmlString(int b);
        bool Hide();
        bool NoPack();
    public:
        void SetAttr(Node* n);
        virtual MATCH_RESULT Match(DatNode* n, RRMessage* msg);
    protected:
        virtual MATCH_RESULT MatchValue(DatNode* n, RRMessage* msg);
        virtual void ProcessDynamicRepeatedNode(RRMessage* msg,
                std::vector<Node*>* newNodes = NULL,
                std::vector<Node*>* delNodes = NULL);
        // 深度拷贝,不拷贝parent
        virtual void DeepCopy(DatNode* n);
    private:
        int ValueCmp(const std::string& v1, const std::string& v2);
    protected:
        std::string m_value;
        std::string m_repeatedNum;
        int m_from;
        int m_match;
        Reg* m_reg;
        const char* m_matchShellCmd;
        EList<std::string> m_matchShellCmdArr;
        //int m_op;
    private:
        static std::map<std::string, std::string> s_fileCache;
};


#endif
