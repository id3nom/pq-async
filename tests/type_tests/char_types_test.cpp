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