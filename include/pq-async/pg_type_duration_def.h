/*
    This file is part of pq-async
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

#ifndef _libpq_async_duration_h
#define _libpq_async_duration_h

#include <cmath>
#include "pg_type_date_def.h"

#include "log.h"
namespace pq_async{

class duration{
    friend std::ostream& operator<<(std::ostream& os, const duration& d);
    friend std::istream& operator>> (std::istream& is, duration& v);

public:
    duration(const std::string& val);
    duration(const char* val);
    duration(long double val): _milliseconds(val) {}
    virtual ~duration(){};
    
    static duration from_seconds(int64_t seconds)
    {
        return duration(seconds * 1000.0L);
    }
    
    std::string to_short_string() const;
    std::string to_long_string() const;
    
    operator interval() const 
    {
        return interval(
            (int64_t)floor(_milliseconds * 1000.0L),
            0, 0
        );
    }
    explicit operator std::string() const;

private:
    long double _milliseconds;

};

std::ostream& operator<<(std::ostream& os, const pq_async::duration& d);
std::istream& operator>> (std::istream& is, duration& v);

}

#endif //_libpq_async_duration_h