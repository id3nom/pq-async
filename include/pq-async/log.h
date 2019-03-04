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

#include <string>
#include <string.h>
#include <stdarg.h>

#include "exceptions.h"
#include "utils.h"

#define PQ_ASYNC_MAX_LOG_LENGTH 4096
#define PQ_ASYNC_MAX_BACKTRACE_FRAMES 20

///////////////////
// helper macros //
///////////////////
#define LOG_NAME "pq-async"

#define pq_async_log_fatal(msg, ...) pq_async::log::fatal( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)
#define pq_async_log_error(msg, ...) \
if(pq_async::log::get_level() >= pq_async::log_level::ll_error) \
    pq_async::log::error( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)
#define pq_async_log_warn(msg, ...) \
if(pq_async::log::get_level() >= pq_async::log_level::ll_warning) \
    pq_async::log::warning( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)
#define pq_async_log_info(msg, ...)  \
if(pq_async::log::get_level() >= pq_async::log_level::ll_info) \
    pq_async::log::info( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)
#define pq_async_log_debug(msg, ...) \
if(pq_async::log::get_level() >= pq_async::log_level::ll_debug) \
    pq_async::log::debug( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)
#define pq_async_log_trace(msg, ...) \
if(pq_async::log::get_level() >= pq_async::log_level::ll_trace) \
    pq_async::log::trace( \
    LOG_NAME, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg, ##__VA_ARGS__)


namespace pq_async{

enum class log_level
{
    ll_fatal = 0,
    ll_error = 1,
    ll_warning = 2,
    ll_info = 3,
    ll_debug = 4,
    ll_trace = 5
};

class log
{
private:
    log();
    virtual ~log();

    static void write(
        log_level level, const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, va_list arg
    );

public:
    
    static std::string get_errno_msg()
    {
        char err_msg[255];
        char* pem = strerror_r(errno, err_msg, 255);
        return std::string(pem);
    }
    
    static log_level get_level(){return s_level;}
    static void set_level(log_level level){s_level = level;}
    static bool is_color_enabled(){return s_color;}
    static void enable_color(bool color){s_color = color;}
    
    static void init(
        const std::string& app_name, int level, bool enable_source
    );
    
    static std::string level_to_str(log_level level);
    static const std::string& machine_id();
    
    static void fatal(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_fatal, log_name, file, line, fn, msg, args);
        va_end(args);
    }
    
    static void error(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        if(s_level < log_level::ll_error)
            return;
        
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_error, log_name, file, line, fn, msg, args);
        va_end(args);
    }
    
    static void warning(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        if(s_level < log_level::ll_warning)
            return;
        
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_warning, log_name, file, line, fn, msg, args);
        va_end(args);
    }
    
    static void info(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        if(s_level < log_level::ll_info)
            return;
        
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_info, log_name, file, line, fn, msg, args);
        va_end(args);
    }
    
    static void debug(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        if(s_level < log_level::ll_debug)
            return;
        
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_debug, log_name, file, line, fn, msg, args);
        va_end(args);
    }
    
    static void trace(
        const char* log_name, const char* file, int line, 
        const char* fn, const char* msg, ...)
    {
        if(s_level < log_level::ll_trace)
            return;
                
        va_list args;
        va_start(args, msg);
        log::write(log_level::ll_trace, log_name, file, line, fn, msg, args);
        va_end(args);
    }

private:
    static std::string s_app_name;
    static log_level s_level;
    static bool s_source;
    static bool s_color;
    
};

} //namespace pq_async

#endif //_libpq_async_log_h
