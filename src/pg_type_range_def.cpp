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

#include <cstring>

#include "pg_type_range_def.h"

namespace pq_async{
	
	static void parse(
		const std::string& in, range_flag& f, std::string& v1, std::string& v2)
	{
		f = range_flag::none;
		if(in == "empty"){
			f = range_flag::empty;
			return;
		}
		
		if((in[0] != '[' && in[0] != '(') || 
			(in[in.size() -1] != ']' && in[in.size() -1] != ')'))
			throw pq_async::exception("Invalid input for range");
		
		if(in[0] == '[')
			f |= range_flag::lb_inc;
		if(in[in.size() -1] == ']')
			f |= range_flag::ub_inc;
		
		size_t cp = in.find(',');
		if(cp == std::string::npos)
			throw pq_async::exception("Invalid input for range");
		
		v1 = in.substr(1, cp -1);
		v2 = in.substr(cp +1, in.size() - (cp +1));
		
		pq_async::trim(v1);
		pq_async::trim(v2);
		
		if(v1.size() == 0)
			f |= range_flag::lb_inf;
		if(v2.size() == 0)
			f |= range_flag::ub_inf;
		
		if(v1.size() == 0 || v2.size() == 0)
			f |= range_flag::contain_empty;
	}
	
	static std::string format(
		const range_flag& f, const std::string& sl, const std::string& su)
	{
		if((int)(f & range_flag::empty) != 0)
			return "empty";
		
		std::string ob = (int)(f & range_flag::lb_inc) != 0 ? "[" : "(";
		std::string cb = (int)(f & range_flag::ub_inc) != 0 ? "]" : ")";
		
		return ob + sl + "," + su + cb;
	}
	
	template<>
	range<int32_t>::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = str_to_num<int32_t>(v1);
		if(v2.size() > 0)
			this->_ub = str_to_num<int32_t>(v1);
	}
	
	template<>
	range<int32_t>::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? num_to_str(this->_lb, false) : "",
			this->has_ubound() ? num_to_str(this->_ub, false) : ""
		);
	}
	
	template<>
	int8range::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = str_to_num<int64_t>(v1);
		if(v2.size() > 0)
			this->_ub = str_to_num<int64_t>(v1);
	}

	template<>
	int8range::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? num_to_str(this->_lb, false) : "",
			this->has_ubound() ? num_to_str(this->_ub, false) : ""
		);
	}
	
	template<>
	numrange::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = numeric(v1.c_str());
		if(v2.size() > 0)
			this->_ub = numeric(v2.c_str());
	}

	template<>
	numrange::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? this->_lb : "",
			this->has_ubound() ? this->_ub : ""
		);
	}


	template<>
	tsrange::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = timestamp::parse(v1);
		if(v2.size() > 0)
			this->_ub = timestamp::parse(v2);
	}

	template<>
	tsrange::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? this->_lb.iso_string() : "",
			this->has_ubound() ? this->_ub.iso_string() : ""
		);
	}

	template<>
	tstzrange::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = timestamp_tz::parse(v1);
		if(v2.size() > 0)
			this->_ub = timestamp_tz::parse(v2);
	}

	template<>
	tstzrange::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? this->_lb.iso_string() : "",
			this->has_ubound() ? this->_ub.iso_string() : ""
		);
	}
	
	template<>
	daterange::range(const char* str)
	{
		std::string s(str);
		pq_async::trim(s);
		
		range_flag f = range_flag::none;
		std::string v1, v2;
		parse(s, f, v1, v2);
		
		if((int)(f & range_flag::empty) != 0){
			this->_flags = f;
			return;
		}
		
		if(v1.size() > 0)
			this->_lb = date::parse(v1);
		if(v2.size() > 0)
			this->_ub = date::parse(v2);
	}
	
	template<>
	daterange::operator std::string() const
	{
		return format(
			this->_flags,
			this->has_lbound() ? this->_lb.iso_string() : "",
			this->has_ubound() ? this->_ub.iso_string() : ""
		);
	}
	

} // ns: pq_async