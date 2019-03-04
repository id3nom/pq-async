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