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

#ifndef _pq_asyncpp_cash_h
#define _pq_asyncpp_cash_h

#include <locale>
#include <cmath>

#include "exceptions.h"
#include "utils.h"
#include "pg_type_numeric_def.h"

namespace pq_async {

class money {
    friend pq_async::money pgval_to_money(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::money& value);
    friend std::ostream& operator<<(std::ostream& s, const money& v);

public:
    money()
        : _val(0),
        _frac_digits(
            std::use_facet<std::moneypunct<char>>(
                money::get_locale()
            ).frac_digits()
        )
    {
    }
    
    money(const money& b)
    {
        _val = b._val;
        _frac_digits = b._frac_digits;
    }
    
    money(const int64_t& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
    }
    money(const int32_t& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
    }
    money(const float& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
    }
    money(const double& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
    }
    
    int64_t frac_digits() const
    {
        return _frac_digits;
    }
    
    bool operator ==(const money& b) const
    {
        money tm(*this);
        tm.to_frac_digits(b._frac_digits);
        return tm._val == b._val;
    }
    
    bool operator !=(const money& b) const
    {
        money tm(*this);
        tm.to_frac_digits(b._frac_digits);
        return tm._val != b._val;
    }
    
    bool operator <(const money& b) const
    {
        if(_frac_digits == b._frac_digits)
            return _val < b._val;
        int64_t aw, ad, bw, bd;
        decimal_parts_to(aw, ad);
        b.decimal_parts_to(bw, bd);
        if(aw == bw)
            return ad < bd;
        return aw < bw;
    }
    
    bool operator >(const money& b) const
    {
        if(_frac_digits == b._frac_digits)
            return _val > b._val;
        int64_t aw, ad, bw, bd;
        decimal_parts_to(aw, ad);
        b.decimal_parts_to(bw, bd);
        if(aw == bw)
            return ad > bd;
        return aw > bw;
    }

    bool operator <=(const money& b) const
    {
        if(_frac_digits == b._frac_digits)
            return _val <= b._val;
        int64_t aw, ad, bw, bd;
        decimal_parts_to(aw, ad);
        b.decimal_parts_to(bw, bd);
        if(aw == bw)
            return ad <= bd;
        return aw <= bw;
    }

    bool operator >=(const money& b) const
    {
        if(_frac_digits == b._frac_digits)
            return _val >= b._val;
        int64_t aw, ad, bw, bd;
        decimal_parts_to(aw, ad);
        b.decimal_parts_to(bw, bd);
        if(aw == bw)
            return ad >= bd;
        return aw >= bw;
    }
    
    money& operator=(const money& b)
    {
        this->_val = b._val;
        this->_frac_digits = b._frac_digits;
        return *this;
    }
    money& operator=(const int64_t& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
        return *this;
    }
    money& operator=(const int32_t& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
        return *this;
    }
    money& operator=(const float& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
        return *this;
    }
    
    money& operator=(const double& b)
    {
        this->_val = val_from_num(b);
        _frac_digits = std::use_facet<std::moneypunct<char>>(
            money::get_locale()
        ).frac_digits();
        return *this;
    }
    
    money operator+(const money& b) const
    {
        money r;
        r._val = this->_val;
        r._frac_digits = this->_frac_digits;
        r.to_frac_digits(b._frac_digits);
        r._val += b._val;
        return r;
    }
    money operator-(const money& b) const
    {
        money r;
        r._val = this->_val;
        r._frac_digits = this->_frac_digits;
        r.to_frac_digits(b._frac_digits);
        r._val -= b._val;
        return r;
    }
    money operator*(const money& b) const
    {
        money r;
        r._val = this->_val;
        r._frac_digits = this->_frac_digits;
        r.to_frac_digits(b._frac_digits);
        
        r._val *= b._val;
        r._val /= r.get_pow();
        return r;
    }
    money operator/(const money& b) const
    {
        money r;
        r._val = this->_val;
        r._frac_digits = this->_frac_digits;
        r.to_frac_digits(b._frac_digits);
        
        r._val /= b._val;
        r._val *= r.get_pow();
        return r;
    }
    money operator%(const money& b) const
    {
        money r;
        r._val = this->_val;
        r._frac_digits = this->_frac_digits;
        r.to_frac_digits(b._frac_digits);

        r._val %= b._val;
        return r;
    }
    money& operator+=(const money& b)
    {
        int64_t tfd = _frac_digits;
        to_frac_digits(b._frac_digits);
        this->_val += b._val;
        to_frac_digits(tfd);
        return *this;
    }
    money& operator-=(const money& b)
    {
        int64_t tfd = _frac_digits;
        to_frac_digits(b._frac_digits);
        this->_val -= b._val;
        to_frac_digits(tfd);
        return *this;
    }
    money& operator*=(const money& b)
    {
        int64_t tfd = _frac_digits;
        to_frac_digits(b._frac_digits);
        this->_val *= b._val;
        this->_val /= get_pow();
        to_frac_digits(tfd);
        return *this;
    }
    money& operator/=(const money& b)
    {
        int64_t tfd = _frac_digits;
        to_frac_digits(b._frac_digits);
        this->_val /= b._val;
        this->_val *= get_pow();
        to_frac_digits(tfd);
        return *this;
    }
    money& operator%=(const money& b)
    {
        int64_t tfd = _frac_digits;
        to_frac_digits(b._frac_digits);
        this->_val %= b._val;
        to_frac_digits(tfd);
        return *this;
    }
    money& operator++() // prefix
    {
        this->_val += val_from_num<int64_t>(1);
        return *this;
    }
    money operator++(int /*unused*/) // postfix
    {
        money t = *this;
        ++(*this);
        return t;
    }
    money& operator--() // prefix
    {
        this->_val -= val_from_num<int64_t>(1);
        return *this;
    }
    money operator--(int /*unused*/) // postfix
    {
        money t = *this;
        --(*this);
        return t;
    }
    
    operator int64_t() const
    {
        return to_num<int64_t>();
    }
    operator int32_t() const
    {
        return to_num<int32_t>();
    }
    operator float() const
    {
        return to_num<float>();
    }
    operator double() const
    {
        return to_num<double>();
    }
    
    
    operator std::string() const
    {
        return to_string();
    }
    
    std::string to_string() const
    {
        // const std::locale l = money::get_locale();
        // const std::moneypunct<char>& mp = 
        //     std::use_facet<std::moneypunct<char> >(l);
        // 
        // return to_string(mp.frac_digits());
        return to_string(_frac_digits);
    }
    
    void decimal_parts_to(
        bool& is_positive, std::string& whole_part, std::string& decim_part
    ) const;
    void decimal_parts_to(int64_t& a, int64_t& b) const;
    
    void to_frac_digits(int64_t frac_digits);
    static money to_frac_digits(const money& m, int64_t frac_digits);
    
    std::string to_string(const std::locale& l) const
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return to_string(mp.frac_digits());
    }
    
    std::string to_string(int frac_digits) const
    {
        money tm(*this);
        tm.to_frac_digits(frac_digits);
        
        int64_t m = std::pow(10L, (int64_t)frac_digits);
        int64_t n = tm._val / m;
        int64_t d = std::abs(_val % m);
        
        std::string ds = md::num_to_str(d, false);
        if(ds.size() < (size_t)frac_digits)
            ds.insert(0, (size_t)frac_digits - ds.size(), '0');
        return md::num_to_str(n, false) + "." + ds;
    }
    
    static const std::locale& get_locale()
    {
        return money::_default_locale;
    }
    static void set_locale(std::locale loc)
    {
        money::_default_locale = loc;
    }
    
    operator numeric() const
    {
        return to_numeric();
    }
    numeric to_numeric() const
    {
        // const std::locale l = money::get_locale();
        // const std::moneypunct<char>& mp = 
        //     std::use_facet<std::moneypunct<char> >(l);
        
        // return to_numeric(mp.frac_digits());
        return to_numeric(_frac_digits);
    }

    numeric to_numeric(const std::locale& l) const
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return to_numeric(mp.frac_digits());
    }
    
    numeric to_numeric(int frac_digits) const
    {
        std::string str = to_string(frac_digits).c_str();
        //str.erase(std::remove(str.begin(), str.end(), '$'), str.end());
        
        return numeric(str.c_str());
    }

    static money from_numeric(const numeric& b)
    {
        const std::locale l = money::get_locale();
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return from_numeric(b, mp.frac_digits());
    }
    
    static money from_numeric(const numeric& b, const std::locale& l)
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return from_numeric(b, mp.frac_digits());
    }
    
    static money from_numeric(const numeric& b, int frac_digits)
    {
        int64_t m = std::pow(10L, (int64_t)frac_digits);
        
        //_val = b._val;
        std::string bs = b;
        money r;
        
        std::size_t dp = bs.find(".");
        if(dp == std::string::npos){
            r._val = (int64_t)b * m;
        } else {
            std::string nbs = bs.substr(0, dp);
            std::string dbs = bs.substr(dp +1);
            if(dbs.size() > (size_t)frac_digits)
                dbs = dbs.substr(0, frac_digits);
            else if(dbs.size() < (size_t)frac_digits)
                dbs.append((size_t)frac_digits - dbs.size(), '0');
            
            r._val = md::str_to_num<int64_t>(nbs) * m;
            r._val += md::str_to_num<int64_t>(dbs);
        }
        r._frac_digits = frac_digits;
        
        return r;
    }


    template<typename T>
    T to_num() const
    {
        // const std::locale l = money::get_locale();
        // const std::moneypunct<char>& mp = 
        //     std::use_facet<std::moneypunct<char> >(l);
        //return to_num<T>(mp.frac_digits());
        
        return to_num<T>(_frac_digits);
    }
    
    template<typename T>
    T to_num(const std::locale& l) const
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return to_num<T>(mp.frac_digits());
    }
    
    template<typename T>
    T to_num(int frac_digits) const
    {
        money tm(*this);
        tm.to_frac_digits(frac_digits);
        T m = std::pow((T)10, (T)frac_digits);
        T v = ((T)tm._val / m);
        return v;
    }
    
    template<typename T>
    static money from_num(const T& b)
    {
        const std::locale l = money::get_locale();
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return from_num<T>(b, mp.frac_digits());
    }
    
    template<typename T>
    static money from_num(const T& b, const std::locale& l)
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return from_num<T>(b, mp.frac_digits());
    }
    
    template<typename T>
    static money from_num(const T& b, int frac_digits)
    {
        money r;
        r._val = val_from_num<T>(b, frac_digits);
        r._frac_digits = frac_digits;
        return r;
    }
    
private:
    
    int64_t get_pow() const
    {
        // const std::locale l = money::get_locale();
        // const std::moneypunct<char>& mp = 
        //     std::use_facet<std::moneypunct<char> >(l);
        //return get_pow(mp.frac_digits());
        return get_pow(_frac_digits);
    }
    static int64_t get_pow(const std::locale& l)
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        return get_pow(mp.frac_digits());
    }
    static int64_t get_pow(int frac_digits)
    {
        return std::pow((int64_t)10, (int64_t)frac_digits);
    }
    
    template<typename T>
    static int64_t val_from_num(const T& b)
    {
        const std::locale l = money::get_locale();
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return val_from_num<T>(b, mp.frac_digits());
    }
    
    template<typename T>
    static int64_t val_from_num(const T& b, const std::locale& l)
    {
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(l);
        
        return val_from_num<T>(b, mp.frac_digits());
    }

    template<typename T>
    static int64_t val_from_num(const T& b, int frac_digits)
    {
        T m = std::pow((T)10, (T)frac_digits);
        return (int64_t)b * (int64_t)m;
    }
    
    int64_t _val;
    int64_t _frac_digits;
    
    static std::locale _default_locale;
};

std::ostream& operator<<(std::ostream& os, const money& v);

} //ns pq_async

#endif //_pq_asyncpp_cash_h