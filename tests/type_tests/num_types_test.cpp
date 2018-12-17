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

class num_types_test
	: public db_test_base
{
public:
};


TEST_F(num_types_test, numeric_test_bin)
{
	try{
		numeric a("12.54");
		ASSERT_THAT((std::string)a, testing::Eq("12.54"));
		
		auto n = db->query_value<numeric>(
			"select '12.54'::numeric "
		);

		n = db->query_value<numeric>(
			"select $1::numeric ", n
		);
		
		if(a != n)
			FAIL();
		
		ASSERT_THAT((std::string)++a, testing::Eq("13.54"));
		ASSERT_THAT((std::string)a++, testing::Eq("13.54"));
		ASSERT_THAT((std::string)a, testing::Eq("14.54"));
		
		a += 2.2;
		a += 34;
		
		//int32_t b = a;
		
		n = db->query_value<numeric>(
			"select $1::numeric ", a
		);
		
		std::cout << "a:" << a << ", n:" << n << std::endl;
		
		if(a != n)
			FAIL();		
		
		std::cout << a << std::endl;
		
	}catch(const std::exception& err){
		std::cout << "Error: " << err.what() << std::endl;
		FAIL();
	}
}




}} //namespace pq_async::tests