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

class data_reader_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists data_reader_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table data_reader_test("
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


TEST_F(data_reader_test, data_reader_sync_test)
{
    try{
        for(const int& i : pq_async::range<int32_t>(0, 5)){
            db->execute(
                "insert into data_reader_test (value) values ($1)",
                "val" + md::num_to_str(i)
            );
            std::cout << i << (i < 4 ? ", ": "\n");
        }
        
        auto reader = db->query_reader("select * from data_reader_test");
        
        while(sp_data_row r = reader->next()){
            std::string v = *(*r)["value"];
            std::cout
                << "id: " << r->as_int64("id")
                << ", value: " << v << std::endl;
        }
        
        ASSERT_THAT(reader->closed(), testing::Eq(true));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(data_reader_test, data_reader_sync_close_test)
{
    try{
        for(const int& i : pq_async::range<int32_t>(0, 5)){
            db->execute(
                "insert into data_reader_test (value) values ($1)",
                "val" + md::num_to_str(i)
            );
            std::cout << i << (i < 4 ? ", ": "\n");
        }
        
        auto reader = db->query_reader("select * from data_reader_test");
        
        while(sp_data_row r = reader->next()){
            std::string v = *(*r)["value"];
            std::cout
                << "id: " << r->as_int64("id")
                << ", value: " << v << std::endl;
            
            if(r->as_int64("id") == 2){
                reader->close();
                break;
            }
        }
        
        ASSERT_THAT(reader->closed(), testing::Eq(true));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(data_reader_test, data_reader_async_test)
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
                        "insert into data_reader_test (value) values ($1)",
                        "val" + md::num_to_str(val),
                        ecb
                    );
                    std::cout << val << (val < 4 ? ", ": "\n");
                }, scb);
            },
            [&](md::callback::async_cb scb){
                db->query_reader("select * from data_reader_test",
                [scb](const md::callback::cb_error& err, sp_data_reader reader){
                    if(err){
                        scb(err);
                        return;
                    }
                    
                    reader->next([scb, reader]
                    (const md::callback::cb_error& err, sp_data_row r){
                        if(err){
                            scb(err);
                            return;
                        }
                        
                        if(!r){
                            ASSERT_THAT(
                                reader->closed(), testing::Eq(true)
                            );
                            scb(nullptr);
                            return;
                        }
                        
                        std::string v = *(*r)["value"];
                        std::cout
                            << "id: " << r->as_int64("id")
                            << ", value: " << v << std::endl;
                    });
                });
            }
        }, [](const md::callback::cb_error& err){
            if(err){
                pq_async::default_logger()->error(MD_ERR(
                    "data_reader_async_test failed with:\n{}", err
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


TEST_F(data_reader_test, data_reader_async_close_test)
{
    try{
        auto eq = md::event_queue::get_default();
        
        eq->series({
            [&](md::callback::async_cb scb){
                pq_async::range<int32_t> r(0, 5);
                eq->each(r.cbegin(), r.cend(),
                [&, scb](const int32_t& val, md::callback::async_cb ecb){
                    db->execute(
                        "insert into data_reader_test (value) values ($1)",
                        "val" + md::num_to_str(val),
                        ecb
                    );
                    std::cout << val << (val < 4 ? ", ": "\n");
                }, scb);
            },
            [&](md::callback::async_cb scb){
                db->query_reader("select * from data_reader_test",
                [scb](const md::callback::cb_error& err, sp_data_reader reader){
                    if(err){
                        scb(err);
                        return;
                    }
                    
                    reader->next([scb, reader]
                    (const md::callback::cb_error& err, sp_data_row r){
                        if(err){
                            scb(err);
                            return;
                        }
                        
                        if(!r){
                            ASSERT_THAT(
                                reader->closed(), testing::Eq(true)
                            );
                            scb(nullptr);
                            return;
                        }
                        
                        std::string v = *(*r)["value"];
                        std::cout
                            << "id: " << r->as_int64("id")
                            << ", value: " << v << std::endl;
                        
                        if(r->as_int64("id") == 2){
                            reader->close();
                            return;
                        }
                        
                    });
                });
            }
        }, [](const md::callback::cb_error& err){
            if(err){
                pq_async::default_logger()->error(MD_ERR(
                    "data_reader_async_test failed with:\n%s", err.c_str()
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