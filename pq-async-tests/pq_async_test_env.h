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

#ifndef _pq_async_test_env_h
#define _pq_async_test_env_h

#include <gmock/gmock.h>
#include <event2/event.h>

#include "pq-async/pq_async.h"

extern int pq_async_log_level;
extern int pq_async_max_pool_size;

namespace pq_async{ namespace tests{

class pq_async_test_env
    : public testing::Environment
{
public:
    pq_async_test_env()
        : testing::Environment(),
        _ev_base(nullptr)
    {
    }
    
    void SetUp() override
    {
        std::cout << "Forcing time_zone initialization" << std::endl;
        auto cz = hhdate::current_zone();
        std::cout << "Current time zone: " << cz->name() << std::endl;
        
        std::cout << "Initializing the log setting" << std::endl;
        // md::log::default_logger()->set_level(
        //     (md::log::log_level)pq_async_log_level
        // );
        auto out_snk = std::make_shared<md::log::sinks::console_sink>(
            true
        );
        out_snk->set_level((md::log::log_level)pq_async_log_level);
        md::log::logger pqlogger =
            std::make_shared<md::log::logger_t>("/", out_snk);
        pqlogger->set_level(
            (md::log::log_level)pq_async_log_level
        );
        md::log::default_logger() = pqlogger;
        
        _ev_base = event_base_new();
        md::event_queue_t::reset(_ev_base);
        std::cout << "Initializing the connection pool" << std::endl;
        pq_async::connection_pool::init(pq_async_max_pool_size, true, true);
    }
    
    void TearDown() override
    {
        std::cout << "Destroying the connection pool" << std::endl;
        pq_async::connection_pool::destroy();
        md::event_queue_t::destroy_default();
        event_base_free(_ev_base);
        _ev_base = nullptr;
    }
    
private:
    event_base* _ev_base;
};

}} //namespace pq_async::tests
#endif //_pq_async_test_env_h