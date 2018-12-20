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

class net_types_test
	: public db_test_base
{
public:
};


TEST_F(net_types_test, cidr_test_bin)
{
	try{
		pq_async::cidr a("192.168.0.0/24");
		ASSERT_THAT((std::string)a, testing::Eq("192.168.0.0/24"));
		
		auto b = db->query_value<cidr>(
			"select '192.168.0.0/24'::cidr "
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
		
		b = db->query_value<cidr>(
			"select $1 ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

		auto c = db->query_value<std::string>(
			"select $1::text ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq(c));
		
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}

TEST_F(net_types_test, inet_test_bin)
{
	try{
		pq_async::inet a("192.168.0.11/24");
		ASSERT_THAT((std::string)a, testing::Eq("192.168.0.11/24"));
		
		auto b = db->query_value<inet>(
			"select '192.168.0.11/24'::inet "
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

		b = db->query_value<inet>(
			"select $1 ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

		auto c = db->query_value<std::string>(
			"select $1::text ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq(c));

		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}

TEST_F(net_types_test, macaddr_test_bin)
{
	try{
		pq_async::macaddr a("00:00:04:46:51:70");
		ASSERT_THAT((std::string)a, testing::Eq("00:00:04:46:51:70"));
		
		auto b = db->query_value<macaddr>(
			"select '00:00:04:46:51:70'::macaddr "
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

		b = db->query_value<macaddr>(
			"select $1 ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}

TEST_F(net_types_test, macaddr8_test_bin)
{
	try{
		int32_t ver = db->query_value<int32_t>("show server_version_num;");
		if(ver < 100000){
			pq_async_log_info(
				"Current PostgreSQL server version do not support macaddr8 data type, minimum required version is 10.0"
			);
			return;
		}
		
		pq_async::macaddr8 a("00:00:04:46:51:70:AA:BB");
		ASSERT_THAT((std::string)a, testing::Eq("00:00:04:46:51:70:aa:bb"));
		
		auto b = db->query_value<macaddr8>(
			"select '00:00:04:46:51:70:AA:BB'::macaddr8 "
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

		b = db->query_value<macaddr8>(
			"select $1 ", a
		);
		ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}


}} //namespace pq_async::tests