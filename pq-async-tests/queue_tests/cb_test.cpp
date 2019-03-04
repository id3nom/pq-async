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

class cb_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists cb_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table cb_test("
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

TEST_F(cb_test, auto_cb_test)
{
    try{
        auto eq = pq_async::event_queue::get_default();
        
        // insert some value
        pq_async::range<int32_t> r(0, 5);
        for(auto v: r)
            db->execute("insert into cb_test (value) values ($1)", 
            std::string("value is ") + num_to_str(v, false),
            run_async // this enable async mode.
            );
        
        db->execute("update cb_test set value = $1 where id = $2",
        "value has changed", 3,
        [](auto err, auto rec_changed){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_THAT(rec_changed, testing::Eq(1));
        });
        
        db->query(
            "select * from cb_test",
        [r](auto err, auto tbl){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_THAT((int32_t)tbl->size(), testing::Eq(r.ubound()));
        });
        
        db->query_single(
            "select * from cb_test where id = 3",
        [](auto err, auto row){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_STREQ(
                row->as_text("value").c_str(),
                "value has changed"
            );
        });
        
        db->query_value<std::string>(
            "select value from cb_test where id = 3",
        [](auto err, auto val){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_STREQ(
                val.c_str(),
                "value has changed"
            );
        });
        
        db->query_reader(
            "select * from cb_test",
        [](auto err, auto reader){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            reader->next([](auto err, auto row){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
                
                if(row){
                    if(row->as_int32("id") == 3)
                        ASSERT_STREQ(
                            row->as_text("value").c_str(),
                            "value has changed"
                        );
                    else{
                        auto val = std::string("value is ") +
                            num_to_str(row->as_int32("id") -1, false);
                        ASSERT_STREQ(
                            row->as_text("value").c_str(),
                            val.c_str()
                        );
                    }
                }
            });
        });
        
        
        eq->run();
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //ns: pq_async::tests