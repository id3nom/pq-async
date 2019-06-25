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

#ifndef _pq_async_db_test_base_h
#define _pq_async_db_test_base_h

#include <gmock/gmock.h>
#include "pq-async/pq_async.h"

extern std::string pq_async_connection_string;

namespace pq_async{ namespace tests{
    
    class db_test_base
        : public testing::Test
    {
    public:
        pq_async::database db;
        
        std::string connection_string(){ return pq_async_connection_string;}
        
        void SetUp() override
        {
            db = pq_async::open(pq_async_connection_string);
        }
        
        void TearDown() override {
            // Code here will be called immediately after each test (right
            // before the destructor).
            db.reset();
        }
    };
    
}} //namespace pq_async::tests

#endif //_pq_async_db_test_base_h
