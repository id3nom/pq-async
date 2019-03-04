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

class net_types_test
    : public db_test_base
{
public:
};


TEST_F(net_types_test, cidr_test_bin)
{
    try{
        pq_async::cidr a("192.168.0.0/24");
        ASSERT_THAT((std::string)a, testing::Eq("192.168.0.0/24"));
        
        auto b = db->query_value<cidr>(
            "select '192.168.0.0/24'::cidr "
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
        b = db->query_value<cidr>(
            "select $1 ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

        auto c = db->query_value<std::string>(
            "select $1::text ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq(c));
        
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(net_types_test, inet_test_bin)
{
    try{
        pq_async::inet a("192.168.0.11/24");
        ASSERT_THAT((std::string)a, testing::Eq("192.168.0.11/24"));
        
        auto b = db->query_value<inet>(
            "select '192.168.0.11/24'::inet "
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

        b = db->query_value<inet>(
            "select $1 ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

        auto c = db->query_value<std::string>(
            "select $1::text ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq(c));

        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(net_types_test, macaddr_test_bin)
{
    try{
        pq_async::macaddr a("00:00:04:46:51:70");
        ASSERT_THAT((std::string)a, testing::Eq("00:00:04:46:51:70"));
        
        auto b = db->query_value<macaddr>(
            "select '00:00:04:46:51:70'::macaddr "
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

        b = db->query_value<macaddr>(
            "select $1 ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(net_types_test, macaddr8_test_bin)
{
    try{
        int32_t ver = db->query_value<int32_t>("show server_version_num;");
        if(ver < 100000){
            PQ_ASYNC_DEF_INFO(
                "Current PostgreSQL server version do not support macaddr8 data type, minimum required version is 10.0"
            );
            return;
        }
        
        pq_async::macaddr8 a("00:00:04:46:51:70:AA:BB");
        ASSERT_THAT((std::string)a, testing::Eq("00:00:04:46:51:70:aa:bb"));
        
        auto b = db->query_value<macaddr8>(
            "select '00:00:04:46:51:70:AA:BB'::macaddr8 "
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

        b = db->query_value<macaddr8>(
            "select $1 ", a
        );
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //namespace pq_async::tests