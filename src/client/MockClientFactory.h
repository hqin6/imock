/**
 * 文件名称：MockClientFactory.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.06
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _MOCKCLIENTFACTORY_H_
#define _MOCKCLIENTFACTORY_H_

#include "MockFactory.h"

class MockClientFactory : public MockFactory
{
    public:
    private:
        BaseMock* MockMap(const std::string& p);
};

#endif
