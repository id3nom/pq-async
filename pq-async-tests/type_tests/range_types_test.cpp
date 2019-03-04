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

#include <gmock/gmock.h>
#include "../db_test_base.h"

namespace pq_async{ namespace tests{

class range_types_test
    : public db_test_base
{
public:
};


TEST_F(range_types_test, int4range_test_bin)
{
    try{
        auto a = db->query_value<int4range>(
            "select '(,2]'::int4range"
        );
        
        std::cout << "a: " << a << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq("(,3)"));
        ASSERT_THROW(a.lbound(), pq_async::exception);
        ASSERT_THAT(a.ubound(), testing::Eq(3));
        
        auto b = a;
        a = db->query_value<int4range>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq("(,3)"));
        ASSERT_THROW(a.lbound(), pq_async::exception);
        ASSERT_THAT(a.ubound(), testing::Eq(3));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(range_types_test, int8range_test_bin)
{
    try{
        auto a = db->query_value<int8range>(
            "select '[2,2]'::int8range"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<int8range>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(range_types_test, numrange_test_bin)
{
    try{
        auto a = db->query_value<numrange>(
            "select '(,2]'::numrange"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<numrange>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(range_types_test, tsrange_test_bin)
{
    try{
        auto a = db->query_value<tsrange>(
            "select '(2018-11-26 10:00,]'::tsrange"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<tsrange>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(range_types_test, tstzrange_test_bin)
{
    try{
        auto a = db->query_value<tstzrange>(
            "select '(2018-11-26 10:00,]'::tstzrange"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<tstzrange>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(range_types_test, daterange_test_bin)
{
    try{
        auto a = db->query_value<daterange>(
            "select '(2018-11-26,]'::daterange"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<daterange>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

}} //namespace pq_async::tests