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

#ifndef _pq_asyncpp_uuid_h
#define _pq_asyncpp_uuid_h

#include "exceptions.h"
#include "utils.h"


namespace pq_async {

class uuid {
public:
	uuid()
		: _uuid{
			0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
		}
	{
	}
	
	uuid(const uint8_t* val, size_t max_len = 16)
		: _uuid{}
	{
		if(max_len > 16)
			max_len = 16;
		std::copy(val, val +max_len, _uuid);
	}
	
	uuid(const uuid& b)
	{
		for(size_t i = 0; i < this->size(); ++i)
			_uuid[i] = b[i];
	}
	
	size_t size() const { return 16;}
	
	uint8_t& operator [](size_t idx)
	{
		if(idx > 16)
			throw pq_async::exception("index is out of bound!");
		return _uuid[idx];
	}
	
	uint8_t operator [](size_t idx) const
	{
		if(idx > 16)
			throw pq_async::exception("index is out of bound!");
		return _uuid[idx];
	}
	
	const uint8_t* data() const
	{
		return &_uuid[0];
	}
	
	operator std::string() const
	{
		return hex_to_str((uint8_t*)_uuid, 4) + "-" +
			hex_to_str((uint8_t*)(_uuid +4), 2) + "-" +
			hex_to_str((uint8_t*)(_uuid +6), 2) + "-" +
			hex_to_str((uint8_t*)(_uuid +8), 2) + "-" +
			hex_to_str((uint8_t*)(_uuid +10), 6);
	}
	
	bool operator ==(const uuid& b) const
	{
		for(size_t i = 0; i < this->size(); ++i)
			if(_uuid[i] != b[i])
				return false;
		return true;
	}
	
	bool operator !=(const uuid& b) const
	{
		return !(*this == b);
	}
	
	bool operator <(const uuid& b) const
	{
		for(size_t i = 0; i < this->size(); ++i)
			if(_uuid[i] > b[i])
				return false;
			else if(_uuid[i] < b[i])
				return true;
		return false;
	}
	
	bool operator >(const uuid& b) const
	{
		return !(*this < b);
	}
	
	bool operator <=(const uuid& b) const
	{
		return (*this < b) || (*this == b);
	}
	
	bool operator >=(const uuid& b) const
	{
		return (*this > b) || (*this == b);
	}
	
	static uuid generate();
	
private:
	uint8_t _uuid[16];
	
};

} //ns pq_async

#endif //_pq_asyncpp_uuid_h