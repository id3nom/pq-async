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

#ifndef _libpq_async_log_h
#define _libpq_async_log_h

#include "pq_async_config.h"
#include "tools-md/tools-md.h"

#if PQ_ASYNC_BUILD_DEBUG
#define PQ_ASYNC_DEF_TRACE(msg, ...) pq_async::default_logger()->trace( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))
#define PQ_ASYNC_DEF_DBG(msg, ...) pq_async::default_logger()->debug( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))
#define PQ_ASYNC_DEF_INFO(msg, ...) pq_async::default_logger()->info( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))

#define PQ_ASYNC_TRACE(log, msg, ...) log->trace( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))
#define PQ_ASYNC_DBG(log, msg, ...) log->debug( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))
#define PQ_ASYNC_INFO(log, msg, ...) log->info( \
    "{} {}:{}\n{}", __PRETTY_FUNCTION__ , __FILE__, __LINE__, \
    fmt::format(msg, ##__VA_ARGS__))

#else
#define PQ_ASYNC_DEF_TRACE(msg, ...) 
#define PQ_ASYNC_DEF_DBG(msg, ...) 
#define PQ_ASYNC_DEF_INFO(msg, ...) 

#define PQ_ASYNC_TRACE(log, msg, ...) 
#define PQ_ASYNC_DBG(log, msg, ...) 
#define PQ_ASYNC_INFO(log, msg, ...) 

#endif

namespace pq_async {

inline md::log::sp_logger default_logger(md::log::sp_logger new_log = nullptr)
{
    static md::log::sp_logger _log = nullptr;
    if(new_log)
        _log = new_log;
    return _log;
}

}//::pq_async

#endif //_libpq_async_log_h
