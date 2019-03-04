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

class char_types_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists char_types_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table char_types_test("
            "id serial primary key, a text, b varchar(20), c char(15)"
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


TEST_F(char_types_test, text_test_bin)
{
    try{
        std::string txt0("testing text");
        auto txt1 = 
            db->query_value<std::string>(
                "select $1", txt0
            );
        
        ASSERT_THAT(txt0, testing::Eq(txt1));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(char_types_test, varchar_test_bin)
{
    try{
        std::string txt0("testing varchar");
        auto txt1 = 
            db->query_value<std::string>(
                "select $1::varchar(20)", txt0
            );
        ASSERT_THAT(txt0, testing::Eq(txt1));
        
        txt1 = 
            db->query_value<std::string>(
                "select $1::varchar(20)", "too long for varchar(20)"
            );
        ASSERT_THAT(txt1, testing::Eq("too long for varchar"));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(char_types_test, char_test_bin)
{
    try{
        std::string txt0("testing char");
        auto txt1 = 
            db->query_value<std::string>(
                "select $1::char(15)", txt0
            );
        
        ASSERT_THAT(txt1, testing::Ne(txt0));
        ASSERT_THAT(txt1, testing::Eq("testing char   "));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(char_types_test, insert_fetch_test_bin)
{
    try{
        std::string txt_a("testing text");
        std::string txt_b("testing varchar");
        std::string txt_c("testing char");
        
        auto id =
            db->query_value<int32_t>(
                "insert into char_types_test (a, b, c) values ($1, $2, $3)"
                "returning id",
                txt_a, txt_b, txt_c
            );
        
        auto r = 
            db->query_single(
                "select * from char_types_test where id = $1", id
            );
        
        auto r_a = r->as_text("a");
        auto r_b = r->as_text("b");
        auto r_c = r->as_text("c");
        
        ASSERT_THAT(r_a, testing::Eq(txt_a));
        ASSERT_THAT(r_b, testing::Eq(txt_b));
        
        ASSERT_THAT(r_c, testing::Ne(txt_c));
        ASSERT_THAT(r_c, testing::Eq("testing char   "));
        
        db->execute("delete from char_types_test where id = $1", id);
        
        // insert too large values
        txt_b = "too long for varchar(20)";
        txt_c = "too long for char(15)";
        
        ASSERT_THROW(
            db->query_value<int32_t>(
                "insert into char_types_test (a, b, c) values ($1, $2, $3)"
                "returning id",
                txt_a, txt_b, txt_c
            ),
            pq_async::exception
        );
        
    }catch(std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}



}} //namespace pq_async::tests