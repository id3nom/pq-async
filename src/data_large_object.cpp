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

#include "data_large_object.h"

namespace pq_async {
    
void data_large_object_t::open(lo_mode m)
{
    if(this->opened())
        throw exception("Alreaddy opened!");
    
    _db->open_connection();
    connection_lock_t conn_lock(_db->_conn);
    _db->begin();
    
    this->_fd = lo_open(
        _db->_conn->conn(),
        this->_oid.data(),
        (int)m
    );
    
    if(this->_fd == -1){
        _db->rollback();
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    
    _opened_read = ((int)m & (int)lo_mode::read) != 0;
    _opened_write = ((int)m & (int)lo_mode::write) != 0;
}

int32_t data_large_object_t::read(char* buf, int32_t len)
{
    if(!this->opened())
        throw exception("Unable to read when large object is closed!");
    
    connection_lock_t conn_lock(_db->_conn);
    int32_t r = lo_read(_db->_conn->conn(), this->_fd, buf, len);
    if(r == -1){
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    return r;
}

int32_t data_large_object_t::write(const char* buf, int32_t len)
{
    if(!this->opened())
        throw exception("Unable to write when large object is closed!");
    
    connection_lock_t conn_lock(_db->_conn);
    int32_t r = lo_write(_db->_conn->conn(), this->_fd, buf, len);
    if(r == -1){
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    return r;
}

int64_t data_large_object_t::tell()
{
    if(!this->opened())
        throw exception("Unable to tell when large object is closed!");
    
    connection_lock_t conn_lock(_db->_conn);
    int64_t r = lo_tell64(_db->_conn->conn(), this->_fd);
    if(r == -1){
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    return r;
}

int64_t data_large_object_t::seek(
    int64_t offset, lo_whence w)
{
    if(!this->opened())
        throw exception("Unable to seek when large object is closed!");
    
    connection_lock_t conn_lock(_db->_conn);
    int64_t r = lo_lseek64(_db->_conn->conn(), this->_fd, offset, (int)w);
    if(r == -1){
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    return r;
}

void data_large_object_t::resize(int64_t new_size)
{
    if(!this->opened())
        throw exception("Unable to resize when large object is closed!");
    
    connection_lock_t conn_lock(_db->_conn);
    int r = lo_truncate64(_db->_conn->conn(), this->_fd, new_size);
    if(r == -1){
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
}

void data_large_object_t::close()
{
    if(!this->opened())
        return;
    
    connection_lock_t conn_lock(_db->_conn);
    int r = lo_close(_db->_conn->conn(), this->_fd);
    this->_fd = -1;
    _opened_read = _opened_write = false;
    
    if(r == -1){
        _db->rollback();
        std::string err_msg = PQerrorMessage(_db->_conn->conn());
        throw pq_async::exception(err_msg);
    }
    _db->commit();
}

void data_large_object_t::unlink()
{
    if(this->opened())
        throw exception("Large object must be closed to be delete!");
    
    _db->open_connection();
    connection_lock_t conn_lock(_db->_conn);
    lo_unlink(_db->_conn->conn(), this->_oid);
}


} // ns: pq_async