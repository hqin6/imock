/**
 * 文件名称：Zlib.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.05.12
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _ZLIB_H_
#define _ZLIB_H_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

class Zlib
{
    public:
        static std::string GZip(const std::string& src);
        static std::string GUnZip(const std::string& src);
        static std::string Zip(const std::string& src);
        static std::string UnZip(const std::string& src);
        static std::string HGUnZip(const std::string& src);
    private:
        static int zcompress(Bytef *data, uLong ndata, 
                Bytef *zdata, uLong *nzdata);
        static int gzcompress(Bytef *data, uLong ndata, 
                Bytef *zdata, uLong *nzdata);
        static int zdecompress(Byte *zdata, uLong nzdata,
                Byte *data, uLong *ndata);
        static int httpgzdecompress(Byte *zdata, uLong nzdata,
                Byte *data, uLong *ndata);
        static int gzdecompress(Byte *zdata, uLong nzdata,
                Byte *data, uLong *ndata);
};


#endif
