#include "Zlib.h"

using namespace std;

/* Compress data */
int Zlib::zcompress(Bytef *data, uLong ndata, 
        Bytef *zdata, uLong *nzdata)
{
    z_stream c_stream;
    int err = 0;

    if(data && ndata > 0)
    {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        if(deflateInit(&c_stream, Z_DEFAULT_COMPRESSION) != Z_OK) return -1;
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out  = *nzdata;
        while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) 
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
        }
        if(c_stream.avail_in != 0) return c_stream.avail_in;
        for (;;) {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
            if(err != Z_OK) return -1;
        }
        if(deflateEnd(&c_stream) != Z_OK) return -1;
        *nzdata = c_stream.total_out;
        return 0;
    }
    return -1;
}

/* Compress gzip data */
int Zlib::gzcompress(Bytef *data, uLong ndata, 
        Bytef *zdata, uLong *nzdata)
{
    z_stream c_stream;
    int err = 0;

    if(data && ndata > 0)
    {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 
                    MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
        c_stream.next_in  = data;
        c_stream.avail_in  = ndata;
        c_stream.next_out = zdata;
        c_stream.avail_out  = *nzdata;
        while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) 
        {
            if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
        }
        if(c_stream.avail_in != 0) return c_stream.avail_in;
        for (;;) {
            if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
            if(err != Z_OK) return -1;
        }
        if(deflateEnd(&c_stream) != Z_OK) return -1;
        *nzdata = c_stream.total_out;
        return 0;
    }
    return -1;
}

/* Uncompress data */
int Zlib::zdecompress(Byte *zdata, uLong nzdata,                 
        Byte *data, uLong *ndata)
{
    int err = 0;
    z_stream d_stream; /* decompression stream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;
    if(inflateInit(&d_stream) != Z_OK) return -1;
    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if(err != Z_OK) return -1;
    }
    if(inflateEnd(&d_stream) != Z_OK) return -1;
    *ndata = d_stream.total_out;
    return 0;
}

/* HTTP gzip decompress */
int Zlib::httpgzdecompress(Byte *zdata, uLong nzdata,                 
        Byte *data, uLong *ndata)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] = 
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;
    if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if(err != Z_OK )
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
                {
                    return -1;
                }
            }
            else return -1;
        }
    }
    if(inflateEnd(&d_stream) != Z_OK) return -1;
    *ndata = d_stream.total_out;
    return 0;
}

/* Uncompress gzip data */
int Zlib::gzdecompress(Byte *zdata, uLong nzdata,                 
        Byte *data, uLong *ndata)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] = 
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;
    if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) return -1;
    //if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if(err != Z_OK )
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
                {
                    return -1;
                }
            }
            else return -1;
        }
    }
    if(inflateEnd(&d_stream) != Z_OK) return -1;
    *ndata = d_stream.total_out;
    return 0;
}
#define ZIP_BUF_MAX_LEN (1024*10)
string Zlib::GZip(const string& src)
{
    static Bytef buf[ZIP_BUF_MAX_LEN];
    uLong len = ZIP_BUF_MAX_LEN; 
    if (gzcompress((Bytef*)src.c_str(), src.size(),
            buf, &len) == 0)
    {
        return string((char*)buf, len);
    }
    return "";
}

string Zlib::GUnZip(const string& src)
{
    static Bytef buf[ZIP_BUF_MAX_LEN];
    uLong len = ZIP_BUF_MAX_LEN; 
    if (gzdecompress((Bytef*)src.c_str(), src.size(),
            buf, &len) == 0)
    {
        return string((char*)buf, len);
    }
    return "";
}

string Zlib::Zip(const string& src)
{
    static Bytef buf[ZIP_BUF_MAX_LEN];
    uLong len = ZIP_BUF_MAX_LEN; 
    if (zcompress((Bytef*)src.c_str(), src.size(),
            buf, &len) == 0)
    {
        return string((char*)buf, len);
    }
    return "";
}

string Zlib::UnZip(const string& src)
{
    static Bytef buf[ZIP_BUF_MAX_LEN];
    uLong len = ZIP_BUF_MAX_LEN; 
    if (zdecompress((Bytef*)src.c_str(), src.size(),
            buf, &len) == 0)
    {
        return string((char*)buf, len);
    }
    return "";
}

string Zlib::HGUnZip(const string& src)
{
    static Bytef buf[ZIP_BUF_MAX_LEN];
    uLong len = ZIP_BUF_MAX_LEN; 
    if (httpgzdecompress((Bytef*)src.c_str(), src.size(),
            buf, &len) == 0)
    {
        return string((char*)buf, len);
    }
    return "";
}

#ifdef _DEBUG_ZSTREAM
#define BUF_SIZE 65535
int main()
{
    char *data = "kjdalkfjdflkjdlkfjdklfjdlkfjlkdjflkdjflddajfkdjfkdfaskf;ldsfk;ldakf;ldskfl;dskf;ld";  
    uLong ndata = strlen(data); 
    Bytef zdata[BUF_SIZE];
    uLong nzdata = BUF_SIZE;
    Bytef  odata[BUF_SIZE];
    uLong nodata = BUF_SIZE;

    memset(zdata, 0, BUF_SIZE);
    //if(zcompress((Bytef *)data, ndata, zdata, &nzdata) == 0)
    if(gzcompress((Bytef *)data, ndata, zdata, &nzdata) == 0)
    {
        fprintf(stdout, "nzdata:%d %s\n", nzdata, zdata);
        memset(odata, 0, BUF_SIZE);
        //if(zdecompress(zdata, ndata, odata, &nodata) == 0)
        if(gzdecompress(zdata, ndata, odata, &nodata) == 0)
        {
            fprintf(stdout, "%d %s\n", nodata, odata);
        }
    }
}
#endif
