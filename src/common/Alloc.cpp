#include "Alloc.h"
#include <stdint.h>

using namespace std;

// 指针 + 大小 + 应用数据
#define _BlockSize(s) (sizeof(uint64_t) + sizeof(int) + s)
// 初始化new的内存
#define _SetSize(b, s) *(int*)((char*)b + sizeof(uint64_t)) = s
// 返回app数据
#define _App(b) ((char*)b + sizeof(uint64_t) + sizeof(int))
// 返回下一个节点地址
#define _NextBlock(b)  (*(uint64_t*)b)
// 得到Block数据
#define _Block(p) ((char*)p - sizeof(int) - sizeof(uint64_t))
// 得到size
#define _Size(b)  (*(int*)((char*)b + sizeof(uint64_t)))
// 设置下一个
#define _SetNext(b, n) _NextBlock(b) = n

map<int, uint64_t> Alloc::s_mapSize;

Alloc::~Alloc()
{
}

/*void* Alloc::operator new(size_t size)
{
    return Get(size);
}

void Alloc::operator delete(void* p)
{
    Release(p);
}*/

void* Alloc::Get(int size)
{
    int ss = _BlockSize(size);
    map<int, uint64_t>::iterator it = s_mapSize.find(size);
    void* b = NULL;
    if (s_mapSize.end() == it || 0 == it->second)
    {
        b = new char[ss];
        _SetSize(b, size);
        return _App(b);
    }
    b = (void*)it->second;
    it->second = _NextBlock(b);
    return _App(b);
}

void Alloc::Release(void* p)
{
    char* b = (char*)_Block(p);
    int size = _Size(b);
    map<int, uint64_t>::iterator it = s_mapSize.find(size);
    if (s_mapSize.end() != it)
    {
	_SetNext(b, it->second);
        it->second = (uint64_t)b;
    }
    else
    {
	_SetNext(b, 0);
        s_mapSize.insert(pair<int, uint64_t>(size, (uint64_t)b));
    }
}

