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