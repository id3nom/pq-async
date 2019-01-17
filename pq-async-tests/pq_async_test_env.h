/*
    This file is part of pq-async
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
        pq_async::log::init(
            std::string("pq_async++_test"),
            pq_async_log_level,
            true
        );
        
        std::cout << "Initializing the connection pool" << std::endl;
        pq_async::connection_pool::init(pq_async_max_pool_size, true, true);
    }
    
    void TearDown() override
    {
        std::cout << "Destroying the connection pool" << std::endl;
        pq_async::connection_pool::destroy();
        pq_async::event_queue::destroy_default();
    }
};

}} //namespace pq_async::tests
#endif //_pq_async_test_env_h