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
                "val" + num_to_str(i)
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
                "val" + num_to_str(i)
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
        auto eq = pq_async::event_queue::get_default();
        
        eq->series({
            [&](async_cb scb){
                pq_async::range<int32_t> r(0, 5);
                eq->each(
                    r.cbegin(), r.cend(),
                [&, scb](const int32_t& val, async_cb ecb){
                    db->execute(
                        "insert into data_reader_test (value) values ($1)",
                        "val" + num_to_str(val),
                        ecb
                    );
                    std::cout << val << (val < 4 ? ", ": "\n");
                }, scb);
            },
            [&](async_cb scb){
                db->query_reader("select * from data_reader_test",
                [scb](const cb_error& err, sp_data_reader reader){
                    if(err){
                        scb(err);
                        return;
                    }
                    
                    reader->next([scb, reader]
                    (const cb_error& err, sp_data_row r){
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
        }, [](const cb_error& err){
            if(err){
                pq_async_log_error(
                    "data_reader_async_test failed with:\n%s", err.c_str()
                );
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
        auto eq = pq_async::event_queue::get_default();
        
        eq->series({
            [&](async_cb scb){
                pq_async::range<int32_t> r(0, 5);
                eq->each(r.cbegin(), r.cend(),
                [&, scb](const int32_t& val, async_cb ecb){
                    db->execute(
                        "insert into data_reader_test (value) values ($1)",
                        "val" + num_to_str(val),
                        ecb
                    );
                    std::cout << val << (val < 4 ? ", ": "\n");
                }, scb);
            },
            [&](async_cb scb){
                db->query_reader("select * from data_reader_test",
                [scb](const cb_error& err, sp_data_reader reader){
                    if(err){
                        scb(err);
                        return;
                    }
                    
                    reader->next([scb, reader]
                    (const cb_error& err, sp_data_row r){
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
        }, [](const cb_error& err){
            if(err){
                pq_async_log_error(
                    "data_reader_async_test failed with:\n%s", err.c_str()
                );
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