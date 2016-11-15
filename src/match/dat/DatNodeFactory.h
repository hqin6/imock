/**
 * 文件名称：DatNodeFactory.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.05.17
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _DATNODEFACTORY_H_
#define _DATNODEFACTORY_H_

#include "tinyxml.h"

class Node;

class DatNodeFactory
{
    public:
        static Node* Get(TiXmlElement* e);
};


#endif
