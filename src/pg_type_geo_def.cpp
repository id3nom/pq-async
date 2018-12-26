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

#include "pg_type_geo_def.h"

namespace pq_async{


    point::point(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        int idx = 0;
        std::string sx;
        std::string sy;
        
        for(size_t i = 0; i < s.size(); ++i){
            if(std::isdigit(s[i]) || s[i] == '.' || s[i] == '-'){
                if(idx == 0)
                    sx += s[i];
                else if(idx == 1)
                    sy += s[i];
            }else if(s[i] == ',')
                ++idx;
        }
        
        if(sx.size() == 0 || sy.size() == 0)
            throw pq_async::exception("Invalid input for type point");
        
        this->_x = str_to_num<double>(sx);
        this->_y = str_to_num<double>(sy);
    }
    point::operator std::string() const
    {
        return "(" + pq_async::num_to_str(this->_x, false) + "," + 
            pq_async::num_to_str(this->_y, false) + ")";
    }
    
    line::line(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc == std::string::npos || (cc != 1 && cc != 3))
            throw pq_async::exception("Invalid input for type line");
        
        if(cc == 3){
            size_t cp = s.find(',');
            cp = s.find(',', cp +1);
            
            line t(
                point(s.substr(0, s.size() - cp).c_str()),
                point(s.substr(cp +1).c_str())
            );
            this->_a = t._a;
            this->_b = t._b;
            this->_c = t._c;
            return;
        }
        
        int idx = 0;
        std::string sa;
        std::string sb;
        std::string sc;
        
        for(size_t i = 0; i < s.size(); ++i){
            if(std::isdigit(s[i]) || s[i] == '.' || s[i] == '-'){
                if(idx == 0)
                    sa += s[i];
                else if(idx == 1)
                    sb += s[i];
                else if(idx == 2)
                    sc += s[i];
            }else if(s[i] == ',')
                ++idx;
        }
        
        if(sa.size() == 0 || sb.size() == 0 || sc.size() == 0)
            throw pq_async::exception("Invalid input for type line");
        
        this->_a = str_to_num<double>(sa);
        this->_b = str_to_num<double>(sb);
        this->_c = str_to_num<double>(sc);
    }
    line::operator std::string() const
    {
        return "{" + 
            pq_async::num_to_str(this->_a, false) + "," + 
            pq_async::num_to_str(this->_b, false) + "," + 
            pq_async::num_to_str(this->_c, false) +
            "}";
    }
    
    lseg::lseg(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc != 3)
            throw pq_async::exception("Invalid input for type lseg");
        
        size_t cp = s.find(',');
        cp = s.find(',', cp +1);
        
        this->_p[0] = point(s.substr(0, s.size() - cp).c_str());
        this->_p[1] = point(s.substr(cp +1).c_str());
    }
    lseg::operator std::string() const
    {
        return "[" + 
            (std::string)this->a() + "," +
            (std::string)this->b() +
            "]";
    }

    box::box(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc != 3)
            throw pq_async::exception("Invalid input for type box");
        
        size_t cp = s.find(',');
        cp = s.find(',', cp +1);
        
        this->_high = point(s.substr(0, s.size() - cp).c_str());
        this->_low = point(s.substr(cp +1).c_str());
    }
    box::operator std::string() const
    {
        return (std::string)this->_high + "," + (std::string)this->_low;
    }

    path::path(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        this->_closed = s[0] == '(';
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc % 2 == 0)
            throw pq_async::exception("Invalid input for type path");
        
        size_t start = 0;
        size_t end = 0;
        while(true){
            start = end +1;
            end = s.find(',', start);
            
            if(end == std::string::npos)
                break;
            
            end = s.find(',', end +1);
            if(end == std::string::npos)
                end = s.size();
            
            std::string sp = s.substr(start, end - start);
            this->_p.push_back(point(sp.c_str()));
            
            if(end == s.size())
                break;
        }
    }
    path::operator std::string() const
    {
        std::string s;
        s += this->_closed ? "(" : "[";
        for(size_t i = 0; i < this->_p.size(); ++i)
            s += (std::string)this->_p[i] +
                (i < this->_p.size() -1 ? "," : "");
        s += this->_closed ? ")" : "]";
        return s;
    }

    polygon::polygon(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc % 2 == 0)
            throw pq_async::exception("Invalid input for type polygon");
        
        s = " " + s;
        size_t start = 0;
        size_t end = 0;
        while(true){
            start = end +1;
            end = s.find(',', start);
            
            if(end == std::string::npos)
                break;
            
            end = s.find(',', end +1);
            if(end == std::string::npos)
                end = s.size();
            
            std::string sp = s.substr(start, end - start);
            this->_p.push_back(point(sp.c_str()));
            
            if(end == s.size())
                break;
        }
    }
    polygon::operator std::string() const
    {
        std::string s;
        s += "(";
        for(size_t i = 0; i < this->_p.size(); ++i)
            s += (std::string)this->_p[i] +
                (i < this->_p.size() -1 ? "," : "");
        s += ")";
        return s;
    }
    
    circle::circle(const char* str)
    {
        std::string s(str);
        pq_async::trim(s);
        
        size_t cc = std::count(s.begin(), s.end(), ',');
        if(cc != 2)
            throw pq_async::exception("Invalid input for type circle");
        
        size_t cp = s.find(',');
        cp = s.find(',', cp +1);
        
        this->_center = point(s.substr(0, s.size() - cp).c_str());
        
        s = s.substr(cp +1);
        std::string sr;
        for(size_t i = 0; i < s.size(); ++i){
            if(std::isdigit(s[i]) || s[i] == '.' || s[i] == '-')
                sr += s[i];
        }
        
        if(sr.size() == 0)
            throw pq_async::exception("Invalid input for type circle");
        
        this->_radius = str_to_num<double>(sr);
    }
    circle::operator std::string() const
    {
        return "<" +
            (std::string)this->_center + "," + 
            pq_async::num_to_str(this->_radius, false) +
            ">";
    }


    std::ostream& operator<<(std::ostream& os, const point& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, point& v)
    {
        std::string s;
        is >> s;
        v = point(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const line& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, line& v)
    {
        std::string s;
        is >> s;
        v = line(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const lseg& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, lseg& v)
    {
        std::string s;
        is >> s;
        v = lseg(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const box& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, box& v)
    {
        std::string s;
        is >> s;
        v = box(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const path& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, path& v)
    {
        std::string s;
        is >> s;
        v = path(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const polygon& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, polygon& v)
    {
        std::string s;
        is >> s;
        v = polygon(s.c_str());
        return is;
    }
    std::ostream& operator<<(std::ostream& os, const circle& v)
    {
        os << (std::string)v;
        return os;
    }
    std::istream& operator>> (std::istream& is, circle& v)
    {
        std::string s;
        is >> s;
        v = circle(s.c_str());
        return is;
    }


} // ns: pq_async