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
        parameters_t p("abc", 1, 2.3, std::array<int, 2>());
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