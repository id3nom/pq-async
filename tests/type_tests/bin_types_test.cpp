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

class bin_types_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists bin_types_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table bin_types_test("
            "id serial primary key, a bytea"
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


TEST_F(bin_types_test, bin_test_bin)
{
    try{
        //std::vector<int8_t>
        //SELECT E'\\xDEADBEEF';
        
        /*
        parameters p("abc", 1, 2.3, std::array<int, 2>());
        for(int i = 0; i < p.size(); ++i)
            std::cout << *(p.types() + i) << std::endl;
        */
    
        int8_t a0[] = {
            (int8_t)222, (int8_t)173, (int8_t)190, (int8_t)239
        };
        
        auto id = db->query_value<int32_t>(
            "insert into bin_types_test (a) values (E'\\\\xDEADBEEF'::bytea) "
            "RETURNING id"
        );
        
        auto r = db->query_single(
            "select * from bin_types_test where id = $1", id
        );
        
        auto a = r->as_bytea("a");
        ASSERT_THAT(a, testing::ElementsAreArray(a0));
        
        db->execute("delete from bin_types_test where id = $1", id);
        
        id = db->query_value<int32_t>(
            "insert into bin_types_test (a) values ($1) "
            "RETURNING id",
            std::vector<int8_t>(a0, a0 + 4)
        );
        
        r = db->query_single(
            "select * from bin_types_test where id = $1", id
        );
        
        a = r->as_bytea("a");
        ASSERT_THAT(a, testing::ElementsAreArray(a0));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
    
}




}} //namespace pq_async::tests