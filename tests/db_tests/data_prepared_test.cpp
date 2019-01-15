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

class data_prepared_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists data_prepared_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table data_prepared_test("
            "id serial primary key, value text"
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


TEST_F(data_prepared_test, data_prepared_test)
{
    try{
        auto dp = db->prepare(
            "abc", "select $1 a, $2 b", false,
            data_type::bigint, data_type::date
        );
        
        auto r = dp->query_single((int64_t)64, date());
        
        std::cout
            << "$1: " << r->as_int64("a")
            << ", $2: " << r->as_date("b")
            << ", $1: " << r->as_int64(0)
            << ", $2: " << r->as_date(1)
            << std::endl;
        
        auto ps_names = db->query("select name from pg_prepared_statements");
        std::cout << "pg_prepared_statements:\n";
        for(auto r : *ps_names)
            std::cout << "name: '" << r->as_text("name") 
                << "'\n";
        std::cout << std::endl;
        
        // ASSERT_THROW(
        // 	dp = db->prepare(
        // 		"abc", "select $1 a, $2 b, $3 c", false,
        // 		data_type::bigint, data_type::date, data_type::text
        // 	),
        // 	pq_async::exception
        // );
        
        ps_names = db->query("select name from pg_prepared_statements");
        std::cout << "pg_prepared_statements:\n";
        for(auto r : *ps_names)
            std::cout << "name: '" << r->as_text("name") 
                << "'\n";
        std::cout << std::endl;
        
        db->deallocate_prepared("abc");
        dp = db->prepare(
            "abc", "select $1 a, $2 b, $3 c", false,
            data_type::bigint, data_type::date, data_type::text
        );
        
        r = dp->query_single((int64_t)64, date(), "test");
        
        std::cout
            << "$1: " << r->as_int64("a")
            << ", $2: " << r->as_date("b")
            << ", $3: " << r->as_date("c")
            << std::endl;
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //namespace pq_async::tests