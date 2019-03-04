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

#include "pg_type_duration_def.h"

#include <regex>
#include <cmath>

namespace pq_async{
    
    const long double s = 1000.0L;
    const long double m = s * 60.0L;
    const long double h = m * 60.0L;
    const long double d = h * 24.0L;
    const long double w = d * 7.0L;
    const long double y = d * 365.25L;
    
    long double text_to_duration(const std::string& val)
    {
        const std::regex re(
            "^((?:\\d+)?\\-?\\d?\\.?\\d+) *(milliseconds?|msecs?|ms|seconds?|secs?|s|minutes?|mins?|m|hours?|hrs?|h|days?|d|weeks?|w|years?|yrs?|y)?$",
            std::regex_constants::icase | std::regex_constants::ECMAScript
            );
        
        std::string tval(val);
        md::trim(tval);
        
        std::smatch sm;
        if(std::regex_match(tval, sm, re)){
            std::string n_str(sm[1].str());
            std::string type_str(sm[2].str());
            type_str = md::lower_case_copy(type_str);
            
            long double n = md::str_to_num<long double>(n_str);
            
            if(type_str == "years" || type_str == "year" || type_str == "yrs" || type_str == "yr" || type_str == "y")
                return n * y;
            if(type_str == "weeks" || type_str == "week" || type_str == "w")
                return n * w;
            if(type_str == "days" || type_str == "day" || type_str == "d")
                return n * d;
            if(type_str == "hours" || type_str == "hour" || type_str == "hrs" || type_str == "hr" || type_str == "h")
                return n * h;
            if(type_str == "minutes" || type_str == "minute" || type_str == "mins" || type_str == "min" || type_str == "m")
                return n * m;
            if(type_str == "seconds" || type_str == "second" || type_str == "secs" || type_str == "sec" || type_str == "s")
                return n * s;
            if(type_str == "milliseconds" || type_str == "millisecond" || type_str == "msecs" || type_str == "msec" || type_str == "ms")
                return n;
            
            throw pq_async::exception("Invalid duration type");
        }
        
        throw pq_async::exception("Invalid duration string");
    }
    
    std::string plural(long double ms_val, long double ms_abs, long double n, const std::string& name)
    {
        bool is_plural = ms_abs >= n * 1.5L;
        return md::num_to_str(std::round(ms_val / n)) + " " + name + (is_plural ? "s" : "");
    }
    
    duration::duration(const std::string& val)
    {
        this->_milliseconds = text_to_duration(val);
    }

    duration::duration(const char* val)
    {
        this->_milliseconds = text_to_duration(val);
    }
    
    std::string duration::to_short_string() const
    {
        long double ms_abs = std::abs(this->_milliseconds);
        if(ms_abs >= d)
            return md::num_to_str(std::round(this->_milliseconds / d)) + "d";
        if(ms_abs >= h)
            return md::num_to_str(std::round(this->_milliseconds / h)) + "h";
        if(ms_abs >= m)
            return md::num_to_str(std::round(this->_milliseconds / m)) + "m";
        if(ms_abs >= s)
            return md::num_to_str(std::round(this->_milliseconds / s)) + "s";
        
        return md::num_to_str(this->_milliseconds) + "ms";
    }
    
    std::string duration::to_long_string() const
    {
        long double ms_abs = std::abs(this->_milliseconds);
        if(ms_abs >= d)
            return plural(this->_milliseconds, ms_abs, d, "day");
        if(ms_abs >= h)
            return plural(this->_milliseconds, ms_abs, h, "hour");
        if(ms_abs >= m)
            return plural(this->_milliseconds, ms_abs, m, "minute");
        if(ms_abs >= s)
            return plural(this->_milliseconds, ms_abs, s, "second");
        
        return md::num_to_str(this->_milliseconds) + " ms";
    }
    
    duration::operator std::string() const
    {
        return this->to_long_string();
    }
    
    std::ostream& operator<<(std::ostream& os, const pq_async::duration& d)
    {
        os << d.to_long_string();
        return os;
    }
    
    std::istream& operator>> (std::istream& is, duration& v)
    {
        std::string s;
        is >> s;
        v = duration(s);
        return is;
    }

}