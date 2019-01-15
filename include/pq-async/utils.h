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

#ifndef _libpq_async_utils_h
#define _libpq_async_utils_h

#include <string>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>
#include <numeric>
#include <type_traits>
#include <functional>
#include <chrono>

#include "traits.h"

namespace pq_async{


template< typename T>
constexpr const T& get_last(const T& v) noexcept
{
    return v;
}
template< typename T, typename... PARAMS>
constexpr const typename select_last<T, PARAMS...>::type&
get_last(const T& /*v*/, const PARAMS&... args) noexcept
{
    return get_last(args...);
}


class stopwatch
{
    typedef std::chrono::high_resolution_clock _hrc;
    typedef std::chrono::duration<double, std::ratio<1>> _sec;
public:
    stopwatch() : _s(_hrc::now()) {}
    void reset() { _s = _hrc::now(); }
    double elapsed() const
    {
        return std::chrono::duration_cast<_sec>(
            _hrc::now() - _s
        ).count();
    }

private:
    std::chrono::time_point<_hrc> _s;
};



inline std::string str_to_lower(const std::string& str)
{
    std::string lstr(str);
    std::transform(
        lstr.begin(), lstr.end(), lstr.begin(),
        [](unsigned char c){ return std::tolower(c); }
    );
    return lstr;
}

inline bool iequals(const std::string& a, const std::string& b)
{
    return (
        (a.size() == b.size()) && 
        std::equal(
            a.begin(), a.end(),
            b.begin(), 
            [](const char& ca, const char& cb)-> bool {
                return ca == cb || std::tolower(ca) == std::tolower(cb);
            }
        )
    );
}

class no_grouping : public std::numpunct_byname<char> {
    std::string do_grouping() const { return ""; }
public:
    no_grouping() : numpunct_byname("") {}
};

template <typename T>
inline std::string num_to_str(T number, bool grouping = true)
{
    std::stringstream ss;
    if(!grouping)
        ss.imbue(std::locale(std::locale(""), new no_grouping));
    
    ss << number;
    return ss.str();
}

template <typename T>
inline T str_to_num(const std::string& text)
{
    std::stringstream ss(text);
    T result;
    return ss >> result ? result : 0;
}

// trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s)
{
    ltrim(s);
    return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s)
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}

template< typename T >
inline std::string join(const T& lst, const std::string& separator)
{
    static_assert(
        std::is_same<typename T::value_type, std::string>::value,
        "Type T must be a container of std::string type"
    );
    return
        std::accumulate(lst.begin(), lst.end(), std::string(),
            [&separator](const std::string& a, const std::string& b) -> std::string { 
                return a + (a.length() > 0 ? separator : "") + b;
            }
        );
}


void swap2(const int16_t* inp, int16_t* outp, bool to_network);
void swap4(const int32_t* inp, int32_t* outp, bool to_network);
void swap8(const int64_t* inp, int64_t* outp, bool to_network);

std::string hex_to_str(const uint8_t* data, int len);

} //namespace pq_async

#endif //_libpq_async_utils_h