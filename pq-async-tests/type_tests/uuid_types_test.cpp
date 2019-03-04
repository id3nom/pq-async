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