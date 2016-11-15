/**
 * 文件名称：File.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.04.28
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _FILE_H_
#define _FILE_H_

#include <string>

class File
{
    public:
        static void Write(const std::string& file, 
                const std::string& msg,
                const std::string& mode ="w" );
};


#endif
