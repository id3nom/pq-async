/*
MIT License

Copyright (c) 2011-2019 Michel Dénommée

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
    long double milliseconds() const { return _milliseconds;}
    
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