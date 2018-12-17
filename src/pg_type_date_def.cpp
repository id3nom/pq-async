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

#include <chrono>
#include "date/date.h"
#include "date/tz.h"
namespace hhdate = date;

#include "pg_type_date_def.h"
namespace pq_async {

	//////////////
	// interval //
	//////////////
	
	interval::interval(const hhdate::sys_time<std::chrono::microseconds>& t)
	{
		
	}
	
	std::string interval::iso_string() const
	{
		int32_t years = this->_m / 12;
		int32_t months = this->_m % 12;
		
		int Y = years;
		int M = months;
		int D = this->_d;
		int h = this->_t.hours().count();
		int m = this->_t.minutes().count();
		int s = this->_t.seconds().count();
		int u = this->_t.subseconds().count();
		
		std::stringstream ss;
		if(u == 0)
			ss << "P" << Y << "Y" << M << "M" << D << "DT"
				<< h << "H" << m << "M" << s << "S";
		else
			ss << "P" << Y << "Y" << M << "M" << D << "DT"
				<< h << "H" << m << "M" << s << "." << u << "S";
		return ss.str();
	}
	
	interval interval::parse(const std::string& s)
	{
		return interval();
	}
	
	//////////
	// date //
	//////////
	
	date::date(int32_t val)
	{
		this->_d = hhdate::sys_days(hhdate::days(val));
		// pg to hhdate alignment...
		if(_d.year() < hhdate::year(1))
			this->_d = this->_d - hhdate::years{1};
	}
	
	
	std::string date::iso_string() const
	{
		return hhdate::format("%Y-%m-%d", this->_d);
	}
	
	date date::parse(const std::string& s)
	{
		return date();
	}
	
	///////////////
	// timestamp //
	///////////////
	
	timestamp::timestamp(int64_t pgticks)
		: _ts(std::chrono::microseconds(pgticks))
	{
		auto daypoint = hhdate::floor<hhdate::days>(this->_ts);
		// calendar date
		auto ymd = hhdate::year_month_day(daypoint);
		
		// pg to hhdate alignment...
		if(ymd.year() < hhdate::year(1)){
			auto tod = hhdate::make_time(_ts - daypoint);
			ymd -= hhdate::years{1};
			this->_ts = 
				hhdate::sys_days(ymd) + 
				tod.hours() + tod.minutes() + 
				tod.seconds() + tod.subseconds();
		}
	}
	
	timestamp::timestamp(const timestamp_tz& tz)
	{
		this->_ts = tz._tsz.get_sys_time();
	}
	
	std::string timestamp::iso_string() const
	{
		return hhdate::format("%Y-%m-%d %T", this->_ts);
	}
	
	timestamp_tz timestamp::as_zone(const char* zone_name) const
	{
		hhdate::local_time<std::chrono::microseconds> lts(
			this->_ts.time_since_epoch()
		);
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, lts);
		return timestamp_tz(ts);
	}

	timestamp_tz timestamp::make_zoned(const char* zone_name) const
	{
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, this->_ts);
		return timestamp_tz(ts);
	}

	timestamp timestamp::parse(const std::string& s)
	{
		timestamp ts;
		std::istringstream is(s);
		is >> hhdate::parse("%Y-%m-%d %T", ts._ts);
		return ts;
	}

	//////////////////
	// timestamp_tz //
	//////////////////
	
	timestamp_tz::timestamp_tz(int64_t pgticks)
		: _tsz(
			hhdate::locate_zone("UTC"),
			hhdate::local_time<std::chrono::microseconds>(
				std::chrono::microseconds(pgticks)
			)
		)
	{
		auto daypoint = 
			hhdate::floor<hhdate::days>(
				this->_tsz.get_sys_time()
			);
		// calendar date
		auto ymd = hhdate::year_month_day(daypoint);
		
		// pg to hhdate alignment...
		if(ymd.year() < hhdate::year(1)){
			auto tod = hhdate::make_time(_tsz.get_sys_time() - daypoint);
			ymd -= hhdate::years{1};
			this->_tsz = hhdate::make_zoned("UTC", 
				hhdate::sys_days(ymd) + 
				tod.hours() + tod.minutes() + 
				tod.seconds() + tod.subseconds()
			);
				// hhdate::zoned_time<std::chrono::microseconds>(
				// 	hhdate::locate_zone("UTC"),
				// 	hhdate::local_time<std::chrono::microseconds>(
				// 		hhdate::sys_days(ymd) + 
				// 		tod.hours() + tod.minutes() + 
				// 		tod.seconds() + tod.subseconds()
				// 	)
				// );
		}
	}
	
	timestamp_tz::timestamp_tz(const timestamp& ts)
	{
		this->_tsz = ts._ts;
	}

	std::string timestamp_tz::iso_string() const
	{
		return hhdate::format("%Y-%m-%d %T%z", this->_tsz);
	}

	timestamp_tz timestamp_tz::as_zone(const char* zone_name) const
	{
		hhdate::local_time<std::chrono::microseconds> lts(
			this->_tsz.get_sys_time().time_since_epoch()
		);
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, lts);
		return timestamp_tz(ts);
	}

	timestamp_tz timestamp_tz::make_zoned(const char* zone_name) const
	{
		auto ts = hhdate::make_zoned(zone_name, this->_tsz);
		return timestamp_tz(ts);
	}

	timestamp_tz timestamp_tz::parse(const std::string& s)
	{
		timestamp_tz ts;
		hhdate::sys_time<std::chrono::microseconds> st;
		std::istringstream is(s);
		is >> hhdate::parse("%Y-%m-%d %T%z", st);
		ts._tsz = st;
		return ts;
	}
	
	
	//////////
	// time //
	//////////

	pq_async::time::time()
	{
		auto now = std::chrono::system_clock::now();
		auto sd = hhdate::floor<hhdate::days>(now);
		auto tod = now - sd;
		this->_tod = 
			hhdate::sys_time<std::chrono::microseconds>(
				hhdate::floor<std::chrono::microseconds>(tod)
			);
	}
	
	pq_async::time::time(int64_t pgticks)
	{
		auto now = std::chrono::microseconds(pgticks);
		auto sd = hhdate::floor<hhdate::days>(now);
		auto tod = now - sd;
		this->_tod = 
			hhdate::sys_time<std::chrono::microseconds>(
				tod
			);
	}
	
	
	pq_async::time::time(const time_tz& tz)
	{
		this->_tod = tz._todz.get_sys_time();
	}
	
	std::string pq_async::time::iso_string() const
	{
		return hhdate::format("%T", this->_tod);
	}
	
	time_tz pq_async::time::as_zone(const char* zone_name) const
	{
		hhdate::local_time<std::chrono::microseconds> lts(
			this->_tod.time_since_epoch()
		);
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, lts);
		return time_tz(ts);
	}
	
	time_tz pq_async::time::make_zoned(const char* zone_name) const
	{
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, this->_tod);
		return time_tz(ts);
	}
	
	pq_async::time pq_async::time::parse(const std::string& s)
	{
		pq_async::time ts;
		std::istringstream is(s);
		is >> hhdate::parse("%T", ts._tod);
		return ts;
	}
	
	
	/////////////
	// time_tz //
	/////////////
	
	time_tz::time_tz()
	{
		auto now = std::chrono::system_clock::now();
		auto sd = hhdate::floor<hhdate::days>(now);
		auto tod = now - sd;
		this->_todz = 
			hhdate::zoned_time<std::chrono::microseconds>(
				hhdate::locate_zone("UTC"),
				hhdate::sys_time<std::chrono::microseconds>(
					hhdate::floor<std::chrono::microseconds>(tod)
				)
			);
	}
	
	time_tz::time_tz(int64_t pgticks)
		//: _tod(std::chrono::microseconds(pgticks))
	{
		auto now = std::chrono::microseconds(pgticks);
		auto sd = hhdate::floor<hhdate::days>(now);
		auto tod = now - sd;
		this->_todz = 
			hhdate::zoned_time<std::chrono::microseconds>(
				hhdate::locate_zone("UTC"),
				hhdate::sys_time<std::chrono::microseconds>(
					tod
				)
			);
	}
	
	
	time_tz::time_tz(const pq_async::time& ts)
	{
		this->_todz = ts._tod;
	}
	
	std::string time_tz::iso_string() const
	{
		return hhdate::format("%T%z", this->_todz);
	}
	
	time_tz time_tz::as_zone(const char* zone_name) const
	{
		hhdate::local_time<std::chrono::microseconds> lts(
			this->_todz.get_sys_time().time_since_epoch()
		);
		hhdate::zoned_time<std::chrono::microseconds> ts =
			hhdate::make_zoned(zone_name, lts);
		return time_tz(ts);
	}

	time_tz time_tz::make_zoned(const char* zone_name) const
	{
		auto ts = hhdate::make_zoned(zone_name, this->_todz);
		return time_tz(ts);
	}
	
	time_tz time_tz::parse(const std::string& s)
	{
		time_tz ts;
		hhdate::sys_time<std::chrono::microseconds> st;
		std::istringstream is(s);
		is >> hhdate::parse("%T%z", st);
		ts._todz = st;
		return ts;
	}
	
	
	
	
	
	
	
	std::ostream& operator<<(std::ostream& os, const interval& v)
	{
		os << v.iso_string();
		return os;
	}
	std::istream& operator>> (std::istream& is, interval& v)
	{
		std::string s;
		is >> s;
		v = interval::parse(s);
		return is;
	}


	std::ostream& operator<<(std::ostream& os, const pq_async::date& v)
	{
		os << v.iso_string();
		return os;
	}

	std::istream& operator>> (std::istream& is, pq_async::date& v)
	{
		std::string s;
		is >> s;
		v = date::parse(s);
		return is;
	}


	std::ostream& operator<<(std::ostream& os, const timestamp& v)
	{
		os << v.iso_string();
		return os;
	}

	std::istream& operator>> (std::istream& is, timestamp& v)
	{
		std::string s;
		is >> s;
		v = timestamp::parse(s);
		return is;
	}


	std::ostream& operator<<(std::ostream& os, const timestamp_tz& v)
	{
		os << v.iso_string();
		return os;
	}

	std::istream& operator>> (std::istream& is, timestamp_tz& v)
	{
		std::string s;
		is >> s;
		v = timestamp_tz::parse(s);
		return is;
	}


	std::ostream& operator<<(std::ostream& os, const time& v)
	{
		os << v.iso_string();
		return os;
	}

	std::istream& operator>> (std::istream& is, time& v)
	{
		std::string s;
		is >> s;
		v = time::parse(s);
		return is;
	}


	std::ostream& operator<<(std::ostream& os, const time_tz& v)
	{
		os << v.iso_string();
		return os;
	}

	std::istream& operator>> (std::istream& is, time_tz& v)
	{
		std::string s;
		is >> s;
		v = time_tz::parse(s);
		return is;
	}

	
	
	
}