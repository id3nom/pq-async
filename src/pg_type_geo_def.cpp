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

#include <cstring>

#include "pg_type_geo_def.h"

namespace pq_async{


    point::point(const char* str)
    {
        std::string s(str);
        md::trim(s);
        
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
        
        this->_x = md::str_to_num<double>(sx);
        this->_y = md::str_to_num<double>(sy);
    }
    point::operator std::string() const
    {
        return "(" + md::num_to_str(this->_x, false) + "," + 
            md::num_to_str(this->_y, false) + ")";
    }
    
    line::line(const char* str)
    {
        std::string s(str);
        md::trim(s);
        
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
        
        this->_a = md::str_to_num<double>(sa);
        this->_b = md::str_to_num<double>(sb);
        this->_c = md::str_to_num<double>(sc);
    }
    line::operator std::string() const
    {
        return "{" + 
            md::num_to_str(this->_a, false) + "," + 
            md::num_to_str(this->_b, false) + "," + 
            md::num_to_str(this->_c, false) +
            "}";
    }
    
    lseg::lseg(const char* str)
    {
        std::string s(str);
        md::trim(s);
        
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
        md::trim(s);
        
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
        md::trim(s);
        
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
        md::trim(s);
        
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
        md::trim(s);
        
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
        
        this->_radius = md::str_to_num<double>(sr);
    }
    circle::operator std::string() const
    {
        return "<" +
            (std::string)this->_center + "," + 
            md::num_to_str(this->_radius, false) +
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