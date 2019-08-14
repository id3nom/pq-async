/*
MIT License

Copyright (c) 2011-2019 Michel Dénommée

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef _libpq_async_data_large_object_h
#define _libpq_async_data_large_object_h

#include "stable_headers.h"

#include "data_common.h"
#include "log.h"

#include "data_connection_pool.h"
#include "database.h"

//extern "C" {
//#include INCLUDE_FILE(LIBPQ_POSTGRESQL_TYPE_INCLUDE_DIR,libpq/libpq-fs.h)
//}
#define INV_WRITE       0x00020000
#define INV_READ        0x00040000

namespace pq_async{

enum class lo_mode
{
    read = INV_READ,
    write = INV_WRITE,
};

enum class lo_whence
{
    seek_start = SEEK_SET,
    seek_cur = SEEK_CUR,
    seek_end = SEEK_END,
};

class data_large_object_t
{
    friend class pq_async::database_t;
    
    data_large_object_t(const database& db, const pq_async::oid& oid)
        : _db(db), _oid(oid), _opened_read(false), _opened_write(false), _fd(-1)
    {
    }
public:
    inline bool opened()
    {
        return _opened_read || _opened_write;
    }
    inline bool opened_read()
    {
        return _opened_read;
    }
    inline bool opened_write()
    {
        return _opened_write;
    }
    
    void open(lo_mode m);
    
    int32_t read(char* buf, int32_t len);
    int32_t write(const char* buf, int32_t len);
    
    int64_t tell();
    int64_t seek(int64_t offset, lo_whence w = lo_whence::seek_cur);
    
    void resize(int64_t new_size);
    
    void close();
    void unlink();
    
private:
    database _db;
    pq_async::oid _oid;
    bool _opened_read;
    bool _opened_write;
    int _fd;
};

} // ns: pq_async

#endif //_libpq_async_data_large_object_h
