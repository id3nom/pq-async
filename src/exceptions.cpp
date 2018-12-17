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

#include "exceptions.h"

namespace pq_async{

	exception::exception(const std::string& message)
		: std::runtime_error(message.c_str()) { }

	exception::exception(const char* message)
		: std::runtime_error(message) { }

	exception::~exception(){}
	
	connection_pool_assign_exception::connection_pool_assign_exception(const std::string& message)
		: pq_async::exception(message) { }

	connection_pool_assign_exception::connection_pool_assign_exception(const char* message)
		: pq_async::exception(message) { }
	
	connection_pool_assign_exception::~connection_pool_assign_exception(){}
	
	
	cb_error cb_error::no_err;


	std::ostream& operator<<(std::ostream& s, const cb_error& v)
	{
		s << std::string(v.c_str());
		return s;
	}


} //namespace pq_async
