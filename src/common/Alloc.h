/**
 * 文件名称：Alloc.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2014.10.13
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */

#ifndef _ALLOC_H
#define _ALLOC_H

#include <map>
#include <new>
#include <stdint.h>
#include <stddef.h>

class Alloc
{
    public:
        virtual ~Alloc();
        static void* Get(int size);
        static void  Release(void* p);
        //static void* operator new(size_t size);
        //static void  operator delete(void* p);
    private:
        static std::map<int, uint64_t> s_mapSize;
};


#endif
