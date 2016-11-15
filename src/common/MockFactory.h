/**
 * 文件名称：MockFactory.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.04.06
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _MOCKFACTORY_H_
#define _MOCKFACTORY_H_

#include <string>
#include "BaseMock.h"

class MockFactory
{
    public:
        virtual ~MockFactory();

        //读取配置文件protocol选项，获取实际对象
        BaseMock* GetMock(const std::string& area);

    protected:
        //字符串对应的实际对象，子类只需要重写该方法
        virtual BaseMock* MockMap(const std::string& p) = 0;
};


#endif
