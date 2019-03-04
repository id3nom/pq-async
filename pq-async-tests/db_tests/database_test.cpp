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

class database_test
    : public db_test_base
{
public:
    void drop_table()
    {
        db->execute("drop table if exists database_test");
    }
    
    void create_table()
    {
        this->drop_table();
        db->execute(
            "create table database_test("
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


TEST_F(database_test, max_connection_sync_test)
{
    try{
        // max con minus the used connection in the the db_test_base
        //size_t max_con = pq_async::connection_pool::get_max_conn() -1;
        size_t nb_con = pq_async::connection_pool::get_max_conn() +20;
        sp_database dbs[nb_con + 1];
        dbs[nb_con] = this->db;
        
        for(size_t i = 0; i < nb_con; ++i)
            dbs[i] = pq_async::database::open(pq_async_connection_string);
        ++nb_con;
        
        for(size_t i = 0; i < nb_con; ++i){
            auto ldb = dbs[i];
            ldb->execute(
                "insert into database_test(value) values ($1)",
                std::string("acb") + md::num_to_str(i)
            );
            std::cout << "execute " << i << " completed" << std::endl;
        }
        
        for(size_t i = 0; i < nb_con; ++i){
            auto ldb = dbs[i];
            auto tbl = ldb->query("select * from database_test");
            std::cout << "select " << i << " completed" << std::endl;
        }

    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(database_test, max_connection_async_test)
{
    try{
        size_t nb_con = pq_async::connection_pool::get_max_conn()*2;
        sp_database dbs[nb_con];
        dbs[0] = this->db;
        dbs[0]->get_strand()->data(0);
        
        std::cout << "running test with, max pool size: "
            << pq_async::connection_pool::get_max_conn()
            << ", db count: " << nb_con << std::endl;
        
        for(size_t i = 1; i < nb_con; ++i){
            dbs[i] = pq_async::database::open(pq_async_connection_string);
            dbs[i]->get_strand()->data(i);
        }
        for(size_t i = 0; i < nb_con; ++i){
            auto ldb = dbs[i];
            pq_async::default_logger()->debug("starting exec {}", i);
            ldb->execute(
                "insert into database_test(value) values ($1)",
                std::string("acb-") + md::num_to_str(i),
                [i](const md::callback::cb_error& err){
                    if(err){
                        std::cout << "exec " << i 
                            << " err: " << err << std::endl;
                        FAIL();
                        return;
                    }
                    pq_async::default_logger()->debug("exec {} completed", i);
                }
            );
        }
        
        for(size_t i = 0; i < nb_con; ++i){
            auto ldb = dbs[i];
            pq_async::default_logger()->debug("starting select {}", i);
            ldb->query(
                "select * from database_test",
                [i](const md::callback::cb_error& err, sp_data_table tbl){
                    if(err){
                        std::cout << "select " << i 
                            << " err: " << err << std::endl;
                        FAIL();
                        return;
                    }
                    pq_async::default_logger()->debug("select {} completed", i);
                }
            );
        }
        
        md::event_queue::get_default()->run();
        
        db->query(
            "select * from database_test",
        [nb_con](const md::callback::cb_error& err, sp_data_table tbl){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_THAT(tbl->size(), testing::Eq(nb_con));
        });

        md::event_queue::get_default()->run();
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(database_test, max_connection_async2_test)
{
    try{
        size_t nb_con = pq_async::connection_pool::get_max_conn()*2;
        sp_database dbs[nb_con];
        dbs[0] = this->db;
        dbs[0]->get_strand()->data(0);
        
        std::cout << "running test with, max pool size: "
            << pq_async::connection_pool::get_max_conn()
            << ", db count: " << nb_con << std::endl;
        
        for(size_t i = 1; i < nb_con; ++i){
            dbs[i] = pq_async::database::open(pq_async_connection_string);
            dbs[i]->get_strand()->data(i);
        }
        
        auto eq = md::event_queue::get_default();
        
        eq->each(dbs, dbs+nb_con,
        [eq](const sp_database& ldb, md::callback::async_cb ecb)->void{
            eq->series({
                [ldb](md::callback::async_cb scb){
                    int i = ldb->get_strand()->data();
                    ldb->execute(
                        "insert into database_test(value) values ($1)",
                        std::string("acb-") + md::num_to_str(i),
                        [scb, i](const md::callback::cb_error& err){
                            if(err){
                                std::cout << "exec " << i 
                                    << " err: " << err << std::endl;
                                scb(err);
                                return;
                            }
                            pq_async::default_logger()->debug(
                                "exec {} completed", i
                            );
                            scb(nullptr);
                        }
                    );
                },
                [ldb](md::callback::async_cb scb){
                    int i = ldb->get_strand()->data();
                    ldb->query(
                        "select * from database_test",
                        [scb, i](const md::callback::cb_error& err, sp_data_table tbl){
                            if(err){
                                std::cout << "select " << i 
                                    << " err: " << err << std::endl;
                                scb(err);
                                return;
                            }
                            pq_async::default_logger()->debug(
                                "select {} completed", i
                            );
                            scb(nullptr);
                        }
                    );
                }
            }, ecb);
        }, [](const md::callback::cb_error& err)->void{
            if(err)
                FAIL();
        });
        
        eq->run();
        
        db->query(
            "select * from database_test",
        [nb_con](const md::callback::cb_error& err, sp_data_table tbl){
                if(err){
                    std::cout << "err: " << err << std::endl;
                    FAIL();
                    return;
                }
            
            ASSERT_THAT(tbl->size(), testing::Eq(nb_con));
        });
        
        eq->run();
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //namespace pq_async::tests