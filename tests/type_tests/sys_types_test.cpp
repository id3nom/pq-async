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
