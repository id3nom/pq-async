/*
    This file is part of libpq-async++
    Copyright (C) 2011-2018 Michel Denommee (and other contributing authors)
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef _libpq_async_exception_h
#define _libpq_async_exception_h

#include "stable_headers.h"

#include <stdexcept>

///////////////////
// error strings //
///////////////////

#define PQ_ASYNC_ERR_NO_SYNC \
"unable to call sync function when database is open in async mode!"
#define PQ_ASYNC_ERR_NO_ASYNC \
"unable to call async function when database is open in sync mode!"

#define PQ_ASYNC_ERR_NO_TEXT_FMT \
"Text format is not supported!"
#define PQ_ASYNC_ERR_DIM_NO_MATCH \
"The app dimension count and server dimension count are not the sames!"


namespace pq_async{

class exception : public std::runtime_error 
{
public:
    //exception(const std::ostream& os);
    exception(const std::string& message);
    exception(const char* message);
    virtual ~exception();
    
    /*
    virtual const char* what() const throw()
    {
    }
    */
};

class connection_pool_assign_exception : public pq_async::exception
{
public:
    connection_pool_assign_exception(const std::string& message);
    connection_pool_assign_exception(const char* message);
    virtual ~connection_pool_assign_exception();
};


class cb_error
{
    friend std::ostream& operator<<(std::ostream& s, const cb_error& v);
public:
    static cb_error no_err;
    
    cb_error(): _has_err(false), _msg("")
    {}
    
    cb_error(nullptr_t /*np*/): _has_err(false), _msg("")
    {}
    
    cb_error(const std::exception& err): _err(err), _has_err(true)
    {
        _msg = std::string(err.what());
        // std::stringstream ss;
        // ss << "Error:\n" << err.what() << std::endl;
        // msg = new char[ss.str().length() +1];
        // std::strncpy(msg, ss.str().c_str(), ss.str().length() +1);
        // msg[ss.str().length()] = 0;
    }
    
    virtual ~cb_error()
    {
        // if(msg)
        // 	delete[] msg;
    }
    
    const std::exception& error() const { return _err;}
    
    explicit operator bool() const
    {
        return _has_err;
    }
    
    explicit operator const char*() const
    {
        if(!_has_err)
            return "No error assigned!";
        return _msg.c_str();
    }
    
    const char* c_str() const
    {
        if(!_has_err)
            return "No error assigned!";
        return _msg.c_str();
    }

private:
    std::exception _err;
    bool _has_err;
    std::string _msg;
};

std::ostream& operator<<(std::ostream& s, const cb_error& v);


} //namespace pq_async
#endif //_libpq_async_exception_h
