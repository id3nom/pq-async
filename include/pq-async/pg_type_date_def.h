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

#ifndef _pq_libasync_date_h
#define _pq_libasync_date_h

#include <chrono>
#include "date/date.h"
#include "date/tz.h"

#include "exceptions.h"

namespace hhdate = date;
namespace pq_async{

class interval;
class date;
class timestamp;
class timestamp_tz;
class time;
class time_tz;

class interval
{
    friend std::ostream& operator<<(std::ostream& os, const interval& v);
    friend std::istream& operator>> (std::istream& is, interval& v);
public:
    interval()
        : _t(std::chrono::microseconds(0)), _d(0), _m(0),
        _is_null(false)
    {
    }
    
    interval(int64_t t, int32_t d, int32_t m)
        : _t(std::chrono::microseconds(t)), _d(d), _m(m),
        _is_null(false)
    {
    }
    
    interval(const hhdate::sys_time<std::chrono::microseconds>& t);
    
    bool is_null() const { return _is_null;}
    static interval null()
    {
        auto v = interval();
        v._is_null = true;
        return v;
    }

    int64_t pgticks() const
    {
        return _t.to_duration().count();
    }
    
    operator hhdate::sys_time<std::chrono::microseconds>() const
    {
        int32_t y = this->_m / 12;
        int32_t m = this->_m % 12;
        
        hhdate::sys_time<std::chrono::microseconds> st(
            this->_t.to_duration()
        );
        
        auto sd = hhdate::floor<hhdate::days>(st);
        auto tod = st - sd;
        
        hhdate::year_month_day ymd = sd;
        if(y > 0)
            ymd += hhdate::years{y};
        if(m > 0)
            ymd += hhdate::months{m};
        
        // if(!ymd.ok())
        // 	ymd = ymd.year()/ymd.month()/ymd.day();
        return hhdate::sys_days{ymd} + tod;
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
    
    hhdate::time_of_day<std::chrono::microseconds> time_of_day() const
    {
        return _t;
    }
    
    int32_t days() const
    {
        return _d;
    }
    
    int32_t months() const
    {
        return _m;
    }
    
    static interval parse(const std::string& s);
    
private:
    hhdate::time_of_day<std::chrono::microseconds> _t;
    int32_t _d;
    int32_t _m;
    bool _is_null;
};

class date
{
    friend std::ostream& operator<<(std::ostream& os, const pq_async::date& v);
    friend std::istream& operator>> (std::istream& is, pq_async::date& v);
public:
    date()
        : _d(
            hhdate::floor<hhdate::days>(
                std::chrono::system_clock::now()
            )
        ),
        _is_null(false)
    {
    }
    
    date(int32_t val);

    date(const hhdate::year_month_day& d)
        : _d(d),
        _is_null(false)
    {
    }
    
    bool is_null() const { return _is_null;}
    static date null()
    {
        auto v = date();
        v._is_null = true;
        return v;
    }
    
    int32_t pgticks() const
    {
        if(_d.year() == hhdate::year(0))
            throw pq_async::exception(
                "Postgresql doesn't support year zero!"
            );
        
        if(_d.year() > hhdate::year(0))
            return ((hhdate::sys_days)_d).time_since_epoch().count();
        
        // hhdate to pg date alignment
        auto td = this->_d + hhdate::years{1};
        return ((hhdate::sys_days)td).time_since_epoch().count();
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
    
    operator hhdate::year_month_day() const
    {
        return this->_d;
    }
    
    static date parse(const std::string& s);
    
private:
    hhdate::year_month_day _d;
    bool _is_null;
};

class timestamp
{
    friend class timestamp_tz;
    friend std::ostream& operator<<(std::ostream& os, const timestamp& v);
    friend std::istream& operator>> (std::istream& is, timestamp& v);
public:
    timestamp()
        : _ts(
            hhdate::floor<std::chrono::microseconds>(
                std::chrono::system_clock::now()
            )
        ),
        _is_null(false)
    {
    }
    
    timestamp(int64_t pgticks);
        
    template< typename Dur >
    explicit timestamp(const hhdate::sys_time<Dur>& ts)
        : _ts(hhdate::floor<Dur>(ts)),
        _is_null(false)
    {
    }
    
    explicit timestamp(const timestamp_tz& tz);
    
    bool is_null() const { return _is_null;}
    static timestamp null()
    {
        auto v = timestamp();
        v._is_null = true;
        return v;
    }
    
    int64_t pgticks() const
    {
        auto daypoint = hhdate::floor<hhdate::days>(this->_ts);
        // calendar date
        auto ymd = hhdate::year_month_day(daypoint);

        if(ymd.year() == hhdate::year(0))
            throw pq_async::exception(
                "Postgresql doesn't support year zero!"
            );
        
        if(ymd.year() > hhdate::year(0))
            return _ts.time_since_epoch().count();
        
        // hhdate to pg alignment...
        auto tod = hhdate::make_time(_ts - daypoint);
        ymd += hhdate::years{1};
        return ((hhdate::sys_time<std::chrono::microseconds>)
            hhdate::sys_days(ymd) + 
            tod.hours() + tod.minutes() + 
            tod.seconds() + tod.subseconds()
        ).time_since_epoch().count();
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
    
    template< typename Dur >
    operator hhdate::sys_time<Dur>() const
    {
        return hhdate::floor< Dur >(this->_ts);
    }
    
    timestamp_tz as_zone(const char* zone_name) const;
    timestamp_tz make_zoned(const char* zone_name) const;
    
    static timestamp parse(const std::string& s);
    
private:
    hhdate::sys_time<std::chrono::microseconds> _ts;
    bool _is_null;
};

class timestamp_tz
{
    friend class timestamp;
    friend std::ostream& operator<<(std::ostream& os, const timestamp_tz& v);
    friend std::istream& operator>> (std::istream& is, timestamp_tz& v);
public:
    timestamp_tz()
        : _tsz(
            hhdate::current_zone(),
            hhdate::floor<std::chrono::microseconds>(
                std::chrono::system_clock::now()
            )
        ),
        _is_null(false)
    {
    }
    
    timestamp_tz(int64_t pgticks);
    
    template< typename Dur >
    explicit timestamp_tz(const hhdate::zoned_time<Dur>& ts)
        : _tsz(ts),
        _is_null(false)
    {
    }
    
    explicit timestamp_tz(const timestamp& ts);
    
    bool is_null() const { return _is_null;}
    static timestamp_tz null()
    {
        auto v = timestamp_tz();
        v._is_null = true;
        return v;
    }
    
    const hhdate::time_zone* zone() const
    {
        return _tsz.get_time_zone();
    }
    
    int64_t pgticks() const
    {
        auto daypoint = hhdate::floor<hhdate::days>(
            this->_tsz.get_sys_time()
        );
        // calendar date
        auto ymd = hhdate::year_month_day(daypoint);
        
        if(ymd.year() == hhdate::year(0))
            throw pq_async::exception(
                "Postgresql doesn't support year zero!"
            );
        
        if(ymd.year() > hhdate::year(0))
            return _tsz.get_sys_time().time_since_epoch().count();
        
        // hhdate to pg alignment...
        auto tod = hhdate::make_time(_tsz.get_sys_time() - daypoint);
        ymd += hhdate::years{1};
        return ((hhdate::sys_time<std::chrono::microseconds>)
            hhdate::sys_days(ymd) + 
            tod.hours() + tod.minutes() + 
            tod.seconds() + tod.subseconds()
        ).time_since_epoch().count();
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
    
    template< typename Dur >
    operator hhdate::sys_time<Dur>() const
    {
        return hhdate::floor< Dur >(this->_tsz.get_sys_time());
    }
    
    template< typename Dur >
    operator hhdate::zoned_time<Dur>() const
    {
        return this->_tsz;
    }
    
    timestamp_tz as_zone(const char* zone_name) const;
    timestamp_tz make_zoned(const char* zone_name) const;
    
    static timestamp_tz parse(const std::string& s);
    
private:
    hhdate::zoned_time<std::chrono::microseconds> _tsz;
    bool _is_null;
};

class time
{
    friend class time_tz;
    friend std::ostream& operator<<(std::ostream& os, const time& v);
    friend std::istream& operator>> (std::istream& is, time& v);
public:
    time();
    
    time(int64_t pgticks);
    
    template< typename Dur >
    explicit time(const hhdate::sys_time<Dur>& tod)
        : _tod(hhdate::floor<Dur>(tod)),
        _is_null(false)
    {
    }
    
    explicit time(const time_tz& tz);
    
    bool is_null() const { return _is_null;}
    static time null()
    {
        auto v = time();
        v._is_null = true;
        return v;
    }
    
    int64_t pgticks() const
    {
        return _tod.time_since_epoch().count();
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
        
    template< typename Dur >
    operator hhdate::sys_time<Dur>() const
    {
        return hhdate::floor< Dur >(this->_tod);
    }
    
    time_tz as_zone(const char* zone_name) const;
    time_tz make_zoned(const char* zone_name) const;
    
    static time parse(const std::string& s);
    
private:
    hhdate::sys_time<std::chrono::microseconds> _tod;
    bool _is_null;
};

class time_tz
{
    friend class time;
    friend std::ostream& operator<<(std::ostream& os, const time_tz& v);
    friend std::istream& operator>> (std::istream& is, time_tz& v);
public:
    time_tz();
    time_tz(int64_t pgticks);
    
    time_tz(const char* zone_name, int64_t pgticks);
    //     : _todz(
    //         hhdate::locate_zone(zone_name),
    //         hhdate::local_time<std::chrono::microseconds>(
    //             std::chrono::microseconds(pgticks)
    //         )
    //     ),
    //     _is_null(false)
    // {
    // }
    
    time_tz(const hhdate::time_zone* zone, int64_t pgticks);
    //     : _todz(
    //         zone,
    //         hhdate::local_time<std::chrono::microseconds>(
    //             std::chrono::microseconds(pgticks)
    //         )
    //     ),
    //     _is_null(false)
    // {
    // }
    
    template< typename Dur >
    explicit time_tz(const hhdate::zoned_time<Dur>& ts)
        : _todz(ts),
        _is_null(false)
    {
    }
    
    explicit time_tz(const time& ts);
    
    bool is_null() const { return _is_null;}
    static time_tz null()
    {
        auto v = time_tz();
        v._is_null = true;
        return v;
    }
    
    const hhdate::time_zone* zone() const
    {
        return _todz.get_time_zone();
    }
    
    int64_t pgticks() const
    {
        return _todz.get_sys_time().time_since_epoch().count();
    }
    
    std::string iso_string() const;
    operator std::string() const { return iso_string();}
    
    template< typename Dur >
    operator hhdate::sys_time<Dur>() const
    {
        return hhdate::floor< Dur >(this->_todz.get_sys_time());
    }
    
    template< typename Dur >
    operator hhdate::zoned_time<Dur>() const
    {
        return hhdate::zoned_time<Dur>(
            this->_todz.get_time_zone(),
            hhdate::floor<Dur>(this->_todz.get_sys_time())
        );
    }
    
    // time_tz as_zone(const char* zone_name) const;
    // time_tz make_zoned(const char* zone_name) const;
    
    template<typename ZoneT>
    time_tz as_zone(ZoneT zone) const
    {
        if(_is_null)
            return time_tz::null();
        
        hhdate::local_time<std::chrono::microseconds> lts(
            this->_todz.get_sys_time().time_since_epoch()
        );
        hhdate::zoned_time<std::chrono::microseconds> ts =
            hhdate::make_zoned(zone, lts);
        return time_tz(ts);
    }
    
    template<typename ZoneT>
    time_tz make_zoned(ZoneT zone) const
    {
        if(_is_null)
            return time_tz::null();
        
        auto ts = hhdate::make_zoned(zone, this->_todz);
        return time_tz(ts);
    }
    
    static time_tz parse(const std::string& s);
    
private:
    hhdate::zoned_time<std::chrono::microseconds> _todz;
    bool _is_null;
};

std::ostream& operator<<(std::ostream& os, const interval& v);
std::istream& operator>> (std::istream& is, interval& v);

std::ostream& operator<<(std::ostream& os, const pq_async::date& v);
std::istream& operator>> (std::istream& is, pq_async::date& v);

std::ostream& operator<<(std::ostream& os, const timestamp& v);
std::istream& operator>> (std::istream& is, timestamp& v);

std::ostream& operator<<(std::ostream& os, const timestamp_tz& v);
std::istream& operator>> (std::istream& is, timestamp_tz& v);

std::ostream& operator<<(std::ostream& os, const time& v);
std::istream& operator>> (std::istream& is, time& v);

std::ostream& operator<<(std::ostream& os, const time_tz& v);
std::istream& operator>> (std::istream& is, time_tz& v);

    
} //namespace pq_async


#endif //_pq_libasync_date_h
