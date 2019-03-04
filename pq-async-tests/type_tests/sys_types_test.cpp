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

class sys_types_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists sys_types_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table sys_types_test("
            "id serial primary key, a oid"
            ");"
        );
    }
    
    void SetUp() override
    {
        db_test_base::SetUp();
        this->create_table();
    }
    
    void TearDown() override
    {
        this->drop_table();
        db_test_base::TearDown();
    }
};


TEST_F(sys_types_test, sys_oid_test_bin)
{
    // max oid:
    //select 4294967295::oid;
    
    /*
    #define 	OIDOID   26
    "select 22630::oid;"
    len: 4
    char[] val{ 0, 0, 88, 102 }
    */
    try{
        u_int32_t i = 4294967295u;
        pq_async::oid o(i);
        ASSERT_THAT((u_int32_t)i, testing::Eq(o));
        
        auto id = db->query_value<int32_t>(
            "insert into sys_types_test (a) values "
            "(4294967295::oid) "
            "RETURNING id"
        );
        
        auto r = db->query_single(
            "select * from sys_types_test where id = $1", id
        );
        
        auto a = r->as_oid("a");
        ASSERT_THAT(a, testing::Eq(o));
        db->execute("delete from sys_types_test where id = $1", id);
        
        id = db->query_value<int32_t>(
            "insert into sys_types_test (a) values ($1) "
            "RETURNING id", o
        );
        r = db->query_single(
            "select * from sys_types_test where id = $1", id
        );
        a = r->as_oid("a");
        ASSERT_THAT(a, testing::Eq(o));
        db->execute("delete from sys_types_test where id = $1", id);
        
        o = 22630u;
        id = db->query_value<int32_t>(
            "insert into sys_types_test (a) values ($1) "
            "RETURNING id", o
        );
        a = db->query_value<pq_async::oid>(
            "select a from sys_types_test where id = $1", id
        );
        ASSERT_THAT(a, testing::Eq(o));
        db->execute("delete from sys_types_test where id = $1", id);
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(sys_types_test, sys_tid_test_bin)
{
    /*
    select ctid, enumtypid, enumsortorder, enumlabel from pg_enum;
    ctid  | enumtypid | enumsortorder | enumlabel 
    -------+-----------+---------------+-----------
    (0,1) |     22630 |             1 | sad
    (0,2) |     22630 |             2 | ok
    (0,3) |     22630 |             3 | happy
    (3 rows)
    
    #define 	TIDOID   27
    "select ctid from pg_enum"
    len: 6
    char[] val{ 0, 0, 0, 0, 0, 1 }
    */
    std::cerr << "NOT IMPLEMENTED!" << std::endl;
    //FAIL();
}

TEST_F(sys_types_test, sys_xid_test_bin)
{
    /*
    #define 	XIDOID   28
    "select xid '22630';"
    len: 4
    char[] val{ 0, 0, 88, 102 }
    */
    std::cerr << "NOT IMPLEMENTED!" << std::endl;
    //FAIL();
}

TEST_F(sys_types_test, sys_cid_test_bin)
{
    /*
    #define 	CIDOID   29
    "select cid '22630'"
    len: 4
    char[] val{ 0, 0, 88, 102 }
    */
    std::cerr << "NOT IMPLEMENTED!" << std::endl;
    //FAIL();
}


TEST_F(sys_types_test, sys_char_test_bin)
{
    /*
    int oid = CHAROID;
    "select char 'a'"
    len: 1
    char[] val{ 97 }
    */
    std::cerr << "NOT IMPLEMENTED!" << std::endl;
    //FAIL();
}

TEST_F(sys_types_test, sys_name_test_bin)
{
    /*
    int oid = NAMEOID;
    "select name 'abc'"
    len: 3
    char[] val{ 97, 98, 99 }
    */
    std::cerr << "NOT IMPLEMENTED!" << std::endl;
    //FAIL();
}


}} //namespace pq_async::tests
