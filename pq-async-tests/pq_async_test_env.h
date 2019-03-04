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
#include "pq-async/pq_async.h"


extern int pq_async_log_level;
extern int pq_async_max_pool_size;

namespace pq_async{ namespace tests{

class pq_async_test_env
    : public testing::Environment
{
public:
    pq_async_test_env()
        : testing::Environment()
    {
    }
    
    void SetUp() override
    {
        std::cout << "Forcing time_zone initialization" << std::endl;
        auto cz = hhdate::current_zone();
        std::cout << "Current time zone: " << cz->name() << std::endl;
        
        std::cout << "Initializing the log setting" << std::endl;
        // pq_async::log::init(
        //     std::string("pq_async++_test"),
        //     pq_async_log_level,
        //     true
        // );
        
        std::cout << "Initializing the connection pool" << std::endl;
        pq_async::connection_pool::init(pq_async_max_pool_size, true, true);
    }
    
    void TearDown() override
    {
        std::cout << "Destroying the connection pool" << std::endl;
        pq_async::connection_pool::destroy();
        md::event_queue::destroy_default();
    }
};

}} //namespace pq_async::tests
#endif //_pq_async_test_env_h