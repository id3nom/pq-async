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

#ifndef _libpq_async_column_h
#define _libpq_async_column_h


#include "data_common.h"

namespace pq_async{

class data_column;
typedef std::shared_ptr< pq_async::data_column > sp_data_column;

class data_column
{
public:
	data_column(int oid, int index, const std::string& name, int fmt);
	
	virtual ~data_column();

	int get_oid() const { return _oid;}
	int get_index() const { return _index;}
	std::string get_name() const { return _name;}
	const char* get_cname() const { return _name.c_str();}
	int get_format() { return _fmt; }

private:
	int _oid;
	int _index;
	std::string _name;
	int _fmt;
};

} //namespace pq_async

#endif //_libpq_async_column_h
