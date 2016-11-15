/**
 * 文件名称：FmtNodeFactory.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.13
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _FMTNODEFACTORY_H_
#define _FMTNODEFACTORY_H_

#include "tinyxml.h"

class Node;

class FmtNodeFactory
{
    public:
        static Node* Get(TiXmlElement* e);
};


#endif
