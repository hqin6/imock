/**
 * 文件名称：MockServerFactory.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.06
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _MOCKSERVERFACTORY_H_
#define _MOCKSERVERFACTORY_H_

#include "MockFactory.h"

class MockServerFactory : public MockFactory
{
    public:
    private:
        BaseMock* MockMap(const std::string& p);
};

#endif
