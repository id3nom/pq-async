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

#ifndef _libpq_async_exception_h
#define _libpq_async_exception_h

#include "stable_headers.h"

#include <stdexcept>

///////////////////
// error strings //
///////////////////

#define PQ_ASYNC_ERR_NO_SYNC \
"unable to call sync function when database_t is open in async mode!"
#define PQ_ASYNC_ERR_NO_ASYNC \
"unable to call async function when database_t is open in sync mode!"

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


// class md::callback::cb_error
// {
//     friend std::ostream& operator<<(std::ostream& s, const md::callback::cb_error& v);
// public:
//     static md::callback::cb_error no_err;
    
//     md::callback::cb_error(): _has_err(false), _msg("")
//     {}
    
//     md::callback::cb_error(nullptr_t /*np*/): _has_err(false), _msg("")
//     {}
    
//     md::callback::cb_error(const std::exception& err): _err(err), _has_err(true)
//     {
//         _msg = std::string(err.what());
//         // std::stringstream ss;
//         // ss << "Error:\n" << err.what() << std::endl;
//         // msg = new char[ss.str().length() +1];
//         // std::strncpy(msg, ss.str().c_str(), ss.str().length() +1);
//         // msg[ss.str().length()] = 0;
//     }
    
//     virtual ~cb_error()
//     {
//         // if(msg)
//         // 	delete[] msg;
//     }
    
//     const std::exception& error() const { return _err;}
    
//     explicit operator bool() const
//     {
//         return _has_err;
//     }
    
//     explicit operator const char*() const
//     {
//         if(!_has_err)
//             return "No error assigned!";
//         return _msg.c_str();
//     }
    
//     const char* c_str() const
//     {
//         if(!_has_err)
//             return "No error assigned!";
//         return _msg.c_str();
//     }

// private:
//     std::exception _err;
//     bool _has_err;
//     std::string _msg;
// };

// std::ostream& operator<<(std::ostream& s, const md::callback::cb_error& v);


} //namespace pq_async
#endif //_libpq_async_exception_h
