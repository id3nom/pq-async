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

TEST_F(queue_test, queue_test)
{
	try{
		int value = 0;
		pq_async::event_queue::get_default()->push_back(
			[&value]() -> void {
				ASSERT_THAT(value, testing::Eq(0));
				++value;
			}
		);
		
		pq_async::event_queue::get_default()->push_back(
			[&value]() -> void {
				ASSERT_THAT(value, testing::Eq(1));
				value += 10;
			}
		);
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(value, testing::Eq(11));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}


TEST_F(queue_test, queue_strand_test)
{
	try{
		auto s1 = event_queue::get_default()->new_strand();
		auto s2 = event_queue::get_default()->new_strand();
		
		stopwatch t;
		
		bool tgl = false;
		int64_t lca = 10000;
		int64_t lcb = 1;
		for(int64_t i = 0; i < lca; ++i){
			for(int64_t j = 0; j < lcb; ++j){
				s1->push_back([&tgl]()->void{
					tgl = !tgl;
					ASSERT_THAT(tgl, testing::Eq(true));
				});
				s2->push_back([&tgl]()->void{
					tgl = !tgl;
					ASSERT_THAT(tgl, testing::Eq(false));
				});
			}
			
			pq_async::event_queue::get_default()->run();
		}
		
		std::cout << "lc: " << (lca * lcb)
			<< ", in " << t.elapsed() << " seconds"
			<< std::endl;
		
		ASSERT_THAT(tgl, testing::Eq(false));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}

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
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(*value, testing::Eq(11));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}

TEST_F(queue_test, queue_db_strand_test_b)
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
						"insert into queue_test (value) values ($1)",
						"val" + num_to_str(val),
						ecb
					);
				}, scb);
			},
			[&](async_cb scb){
				db->query("select * from queue_test",
				[scb](const cb_error& err, sp_data_table tbl){
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
		}, [](const cb_error& err){
			if(err){
				pq_async_log_error(
					"queue_db_strand_test_b failed with:\n%s", err.c_str()
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


TEST_F(queue_test, queue_async_each_test)
{
	try{
		std::vector<int> v{1,0,10};
		int value = 0;
		
		pq_async::event_queue::get_default()->each(v.begin(), v.end(),
		[&value](const int& val, async_cb ecb)->void{
			value += val;
			ecb(nullptr);
		}, [](const cb_error& err)->void{
			if(err)
				FAIL();
		});
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(value, testing::Eq(11));
		
		pq_async::async::each(
			pq_async::event_queue::get_default(), v.begin(), v.end(),
		[&value](const int& val, async_cb ecb)->void{
			value += val;
			ecb(nullptr);
		}, [](const cb_error& err)->void{
			if(err)
				FAIL();
		});
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(value, testing::Eq(22));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}


TEST_F(queue_test, queue_async_series_test)
{
	try{
		int value = 0;
		
		pq_async::event_queue::get_default()->series({
			[&value](async_cb scb)->void{
				value += 1;
				scb(nullptr);
			},
			[&value](async_cb scb)->void{
				value += 0;
				scb(nullptr);
			},
			[&value](async_cb scb)->void{
				value += 10;
				scb(nullptr);
			}
			
		}, [&value](const cb_error& err)->void{
			if(err)
				FAIL();
			ASSERT_THAT(value, testing::Eq(11));
		});
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(value, testing::Eq(11));
		
		pq_async::async::series(
		pq_async::event_queue::get_default(), {
			[&value](async_cb scb)->void{
				value += 1;
				scb(nullptr);
			},
			[&value](async_cb scb)->void{
				value += 0;
				scb(nullptr);
			},
			[&value](async_cb scb)->void{
				value += 10;
				scb(nullptr);
			}
			
		}, [&value](const cb_error& err)->void{
			if(err)
				FAIL();
			ASSERT_THAT(value, testing::Eq(22));
		});
		
		pq_async::event_queue::get_default()->run();
		ASSERT_THAT(value, testing::Eq(22));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}



TEST_F(queue_test, queue_async_multithread_test)
{
	#ifdef PQ_ASYNC_THREAD_SAFE
	try{
		stopwatch t;
		std::atomic<int> value(0);
		
		int cnt = 1000;
		int rnd = 10;
		for(int i = 0; i < cnt; ++i)
			pq_async::event_queue::get_default()->series({
				[&value](async_cb scb)->void{
					++value;
					scb(nullptr);
				},
				[&value](async_cb scb)->void{
					++value;
					scb(nullptr);
				},
				[&value](async_cb scb)->void{
					++value;
					scb(nullptr);
				}
				
			}, [&value](const cb_error& err)->void{
				if(err)
					FAIL();
			});
		
		std::vector<std::thread> threads;
		int cc = std::thread::hardware_concurrency();
		for(int i = 0; i < cc; ++i)
			threads.emplace_back(
				std::thread([&t, &value, rnd, cnt, cc](){
					t.reset();
					auto eq = pq_async::event_queue::get_default();
					int rc = rnd;
					while(true){
						size_t size = eq->local_size();
						if(size == 0){
							if(rc-- == 0)
								break;
							for(int i = 0; i < cnt; ++i)
								eq->series({
									[&value](async_cb scb)->void{
										++value;
										scb(nullptr);
									},
									[&value](async_cb scb)->void{
										++value;
										scb(nullptr);
									},
									[&value](async_cb scb)->void{
										++value;
										scb(nullptr);
									}
									
								}, [&value](const cb_error& err)->void{
									if(err)
										FAIL();
								});
						}
						
						auto jc = std::ceil((double)size / (double)cc / 100.0);
						if(jc < 1)
							jc = 1;
						eq->run_n(jc);
						//eq->run_n(15);
					}
				})
			);
		
		for(auto& th : threads){
			if(th.joinable())
				th.join();
		}
		
		std::cout << "v: " << value.load()
			<< ", in " << t.elapsed() << " seconds"
			<< std::endl;
		
		ASSERT_THAT(value.load(), testing::Eq((cnt*3+(cnt*cc*rnd*3))));
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
	#else
		std::cerr << "multi-thread test require the library to be build with "
			<< "PQ_ASYNC_THREAD_SAFE flag enabled"
			<< std::endl;
	#endif
}



}} //namespace pq_async::tests