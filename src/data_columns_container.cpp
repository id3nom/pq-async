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

#include "data_columns_container.h"

namespace pq_async{

data_columns_container::data_columns_container()
{
	pq_async_log_trace("ptr: %p", this);
}

data_columns_container::~data_columns_container()
{
	pq_async_log_trace("ptr: %p", this);
}

pq_async::sp_data_column pq_async::data_columns_container::get_col(int idx)
{
	return (*this)[idx];
}

pq_async::sp_data_column pq_async::data_columns_container::get_col(
	const char* col_name)
{
	return this->get_col(this->get_col_index(col_name));
}


} //namespace pq_async