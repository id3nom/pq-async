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

class queue_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists queue_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table queue_test("
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



TEST_F(queue_test, queue_db_strand_test)
{
    try{
        auto value = std::make_shared<int>(0);
        db->query_value<int32_t>(
            "select 1", 
            [value](int32_t val)-> void {
                ASSERT_THAT(*value, testing::Eq(0));
                *value += val;
            }
        );
        
        db->query_value<int32_t>(
            "select 10 from pg_sleep(3)", 
            [value](int32_t val)-> void {
                ASSERT_THAT(*value, testing::Eq(1));
                *value += val;
            }
        );
        std::cout << "PostgreSQL query will sleep 3 seconds" << std::endl;
        
        md::event_queue::get_default()->run();
        ASSERT_THAT(*value, testing::Eq(11));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(queue_test, queue_db_strand_test_b)
{
    try{
        auto eq = md::event_queue::get_default();
        
        eq->series({
            [&](md::callback::async_cb scb){
                pq_async::range<int32_t> r(0, 5);
                eq->each(
                    r.cbegin(), r.cend(),
                [&, scb](const int32_t& val, md::callback::async_cb ecb){
                    db->execute(
                        "insert into queue_test (value) values ($1)",
                        "val" + md::num_to_str(val),
                        ecb
                    );
                }, scb);
            },
            [&](md::callback::async_cb scb){
                db->query("select * from queue_test",
                [scb](const md::callback::cb_error& err, data_table tbl){
                    if(err){
                        scb(err);
                        return;
                    }
                    
                    for(auto r : *tbl){
                        std::string v = *(*r)["value"];
                        std::cout
                            << "id: " << r->as_int64("id")
                            << ", value: " << v << std::endl;
                    }
                    
                });
            }
        }, [](const md::callback::cb_error& err){
            if(err){
                pq_async::default_logger()->error(MD_ERR(
                    "queue_db_strand_test_b failed with:\n%s", err.c_str()
                ));
                FAIL();
            }
        });
        
        eq->run();
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}



}} //namespace pq_async::tests