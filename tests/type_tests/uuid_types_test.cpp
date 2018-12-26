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

class uuid_types_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists uuid_types_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table uuid_types_test("
            "id serial primary key, a uuid"
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


TEST_F(uuid_types_test, uuid_test_bin)
{
    try{
        u_int8_t a0[] = {
            0x6b, 0xe8, 0xd9, 0x3c,
            0xe4, 0x58, 
            0x11, 0xe8, 
            0xbd, 0x0e, 
            0x1c, 0x87, 0x2c, 0x56, 0x1f, 0xcc
        };
        uuid u(a0);
        std::string su("6be8d93c-e458-11e8-bd0e-1c872c561fcc");
        
        ASSERT_THAT((std::string)u, testing::Eq(su));
        
        auto id = db->query_value<int32_t>(
            "insert into uuid_types_test (a) values "
            "('6be8d93c-e458-11e8-bd0e-1c872c561fcc'::uuid) "
            "RETURNING id"
        );
        
        auto r = db->query_single(
            "select * from uuid_types_test where id = $1", id
        );
        
        auto a = r->as_uuid("a");
        ASSERT_THAT(a, testing::Eq(u));
        
        db->execute("delete from uuid_types_test where id = $1", id);
        
        id = db->query_value<int32_t>(
            "insert into uuid_types_test (a) values ($1) "
            "RETURNING id", u
        );
        
        r = db->query_single(
            "select * from uuid_types_test where id = $1", id
        );
        
        a = r->as_uuid("a");
        ASSERT_THAT(a, testing::Eq(u));
        db->execute("delete from uuid_types_test where id = $1", id);
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}




}} //namespace pq_async::tests