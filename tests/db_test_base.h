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

#ifndef _pq_async_db_test_base_h
#define _pq_async_db_test_base_h

#include <gmock/gmock.h>
#include "src/pq_async.h"

extern std::string pq_async_connection_string;

namespace pq_async{ namespace tests{
    
    class db_test_base
        : public testing::Test
    {
    public:
        pq_async::sp_database db;
        
        std::string connection_string(){ return pq_async_connection_string;}
        
        void SetUp() override
        {
            db = pq_async::database::open(pq_async_connection_string);
        }
        
        void TearDown() override {
            // Code here will be called immediately after each test (right
            // before the destructor).
            db.reset();
        }
    };
    
}} //namespace pq_async::tests

#endif //_pq_async_db_test_base_h
