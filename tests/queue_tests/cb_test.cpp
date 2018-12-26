/*
    This file is part of libpq-async++
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