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

#ifndef _pq_asyncpp_geo_h
#define _pq_asyncpp_geo_h

#include <vector>
#include <cmath>

#include "exceptions.h"
#include "utils.h"

namespace pq_async {

class parameter;

class line;
class lseg;
class box;
class path;
class polygon;
class circle;

class point {
    friend lseg;
    
    friend pq_async::point pgval_to_point(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::point& value);
    friend pq_async::parameter* new_parameter(const pq_async::line& value);
    friend pq_async::parameter* new_parameter(const pq_async::lseg& value);
    friend pq_async::parameter* new_parameter(const pq_async::box& value);
    friend pq_async::parameter* new_parameter(const pq_async::path& value);
    friend pq_async::parameter* new_parameter(const pq_async::polygon& value);
    friend pq_async::parameter* new_parameter(const pq_async::circle& value);
    
    friend std::ostream& operator<<(std::ostream& os, const point& v);
    friend std::istream& operator>> (std::istream& is, point& v);
    
public:
    point()
        : _x(0.0), _y(0.0)
    {
    }
    
    point(double x, double y)
        : _x(x), _y(y)
    {
    }
    
    point(const char* str);
    operator std::string() const;
    
    
    inline double x() const { return _x;}
    inline double y() const { return _y;}
    inline double& x() { return _x;}
    inline double& y() { return _y;}
    inline void x(double val){ _x = val;}
    inline void y(double val){ _y = val;}
    
    bool operator ==(const point& b) const
    {
        return cmp(*this, b) == 0;
    }
    
    bool operator !=(const point& b) const
    {
        return cmp(*this, b) != 0;
    }
    
    bool operator <(const point& b) const
    {
        return cmp(*this, b) < 0;
    }
    
    bool operator >(const point& b) const
    {
        return cmp(*this, b) > 0;
    }
    
    bool operator <=(const point& b) const
    {
        return cmp(*this, b) <= 0;
    }
    
    bool operator >=(const point& b) const
    {
        return cmp(*this, b) >= 0;
    }
    
    point operator+(const point& b) const
    {
        point r;
        r._x = this->_x + b._x;
        r._y = this->_y + b._y;
        return r;
    }
    point operator-(const point& b) const
    {
        point r;
        r._x = this->_x - b._x;
        r._y = this->_y - b._y;
        return r;
    }
    point operator*(const point& b) const
    {
        point r;
        r._x = this->_x * b._x;
        r._y = this->_y * b._y;
        return r;
    }
    point operator/(const point& b) const
    {
        point r;
        r._x = this->_x / b._x;
        r._y = this->_y / b._y;
        return r;
    }
    point& operator+=(const point& b)
    {
        this->_x += b._x;
        this->_y += b._y;
        return *this;
    }
    point& operator-=(const point& b)
    {
        this->_x -= b._x;
        this->_y -= b._y;
        return *this;
    }
    point& operator*=(const point& b)
    {
        this->_x *= b._x;
        this->_y *= b._y;
        return *this;
    }
    point& operator/=(const point& b)
    {
        this->_x /= b._x;
        this->_y /= b._y;
        return *this;
    }

    point operator-() const
    {
        return point(-_x, -_y);
    }
    
    point operator*(const double& s) const
    {
        return point(_x * s, _y * s);
    }
    point& operator*=(const double& s)
    {
        _x *= s;
        _y *= s;
        return *this;
    }
    point operator/(const double& s) const
    {
        return point(_x / s, _y / s);
    }
    point& operator/=(const double& s)
    {
        _x /= s;
        _y /= s;
        return *this;
    }
    
    double dot(const point& b) const
    {
        return dot(*this, b);
    }
    
    double cross(const point& b) const
    {
        return cross(*this, b);
    }
    
    double magnitude() const
    {
        return std::sqrt((_x * _x) + (_y * _y));
    }
    
    point normal() const
    {
        double m = this->magnitude();
        return point(_x / m, _y / m);
    }
    
    point perpendicular() const
    {
        return point(_y, -_x);
    }
    
    static double dot(const point& a, const point& b)
    {
        return (a._x * b._x) + (a._y * b._y);
    }
    
    static double cross(const point& a, const point& b)
    {
        return (a._x * b._x) - (a._y * b._y);
    }
    
    static int cmp(const point& a, const point& b)
    {
        if(a._x == b._x && a._y == b._y)
            return 0;
        
        double av = (a._x * a._x + a._y * a._y);
        double bv = (b._x * b._x + b._y * b._y);
        if(av == bv)
            return 0;
        if(av < bv)
            return -1;
        return 1;
    }
    
private:
    double _x;
    double _y;
};

class line {
    friend pq_async::line pgval_to_line(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::line& value);
    
    friend std::ostream& operator<<(std::ostream& os, const line& v);
    friend std::istream& operator>> (std::istream& is, line& v);
    
public:
    line()
        : _a(0.0), _b(0.0), _c(0.0)
    {
    }
    
    line(double a, double b, double c)
        : _a(a), _b(b), _c(c)
    {
    }

    line(const point& a, const point& b)
    {
        double x1 = a.x();
        double y1 = a.y();
        double x2 = b.x();
        double y2 = b.y();
        
        double m = (y2 - y1) / (x2 - x1);
        double i = y1 - m * x1;
        
        _a = m;
        _c = i;
        _b = ((m * x1) + _c) / -y1;
        
        if(_a == -0)
            _a = 0;
        if(_b == -0)
            _b = 0;
        if(_c == -0)
            _c = 0;
    }

    
    line(const char* str);
    operator std::string() const;
    
    double a() const { return _a;}
    double b() const { return _b;}
    double c() const { return _c;}
    double& a() { return _a;}
    double& b() { return _b;}
    double& c() { return _c;}
    void a(double val){ _a = val;}
    void b(double val){ _b = val;}
    void c(double val){ _c = val;}
    
    
    point point_from_x(double x) const
    {
        if(_a == 0.0 && _b == -1.0)
            return point(x, _c);
        
        double y = (-(_a * x) - (_c)) / _b;
        return point(x, y);
    }
    
    point point_from_y(double y) const
    {
        if(_a == -1.0 && _b == 0.0)
            return point(_c, y);
        
        double x = (-(_b * y) - (_c)) / _a;
        return point(x, y);
    }
    
private:
    double _a;
    double _b;
    double _c;
};

class lseg {
    friend pq_async::lseg pgval_to_lseg(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::lseg& value);
    
    friend std::ostream& operator<<(std::ostream& os, const lseg& v);
    friend std::istream& operator>> (std::istream& is, lseg& v);
    
public:
    lseg()
        : _p{}
    {
    }
    
    lseg(const point& a, const point& b)
        : _p{a, b}
    {
    }

    lseg(const char* str);
    operator std::string() const;
    
    point a() const { return _p[0];}
    point b() const { return _p[1];}
    point& a() { return _p[0];}
    point& b() { return _p[1];}
    
    void a(const point& val){ _p[0] = val;}
    void b(const point& val){ _p[1] = val;}
    
    
    bool intersect(const lseg& b) const
    {
        return intersect(*this, b);
    }
    
    static bool intersect(const lseg& a, const lseg& b)
    {
        point p = a.a();
        point r = a.b() - a.a();
        point q = b.a();
        point s = b.b() - b.a();
        
        double t = point::cross((q - p), s) / point::cross(r, s);
        double u = point::cross((q - p), r) / point::cross(r, s);
        return (0.0 <= t && t <= 1.0) && (0.0 <= u && u <= 1.0);
    }
    
    static point get_intersect(const lseg& a, const lseg& b)
    {
        point aa = a.a();
        point ab = a.b();
        point ba = b.a();
        point bb = b.b();
        
        double px = (aa._x*ab._y - aa._y*ab._x)*(ba._x - bb._x) -
            (ba._x*bb._y - ba._y*bb._x)*(aa._x - ab._x);
        double py = (aa._x*ab._y - aa._y*ab._x)*(ba._y - bb._y) -
            (ba._x*bb._y - ba._y*bb._x)*(aa._y - ab._y);
        double denominator = (aa._x - ab._x)*(ba._y - bb._y) -
            (aa._y - ab._y)*(ba._x - bb._x);

        return point(px / denominator, py / denominator);
    }
    
private:
    point _p[2];
};

class box {
    friend pq_async::box pgval_to_box(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::box& value);
    
    friend std::ostream& operator<<(std::ostream& os, const box& v);
    friend std::istream& operator>> (std::istream& is, box& v);
    
public:
    box()
        : _high(), _low()
    {
    }
    
    box(const point& h, const point& l)
        : _high(h), _low(l)
    {
    }
    
    box(const char* str);
    operator std::string() const;
    
    point high() const { return _high;}
    point low() const { return _low;}
    point& high() { return _high;}
    point& low() { return _low;}
    
    void high(const point& val){ _high = val;}
    void low(const point& val){ _low = val;}
    
private:
    point _high;
    point _low;
};

class path {
    friend pq_async::path pgval_to_path(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::path& value);
    
    friend std::ostream& operator<<(std::ostream& os, const path& v);
    friend std::istream& operator>> (std::istream& is, path& v);
    
public:
    path()
    {
    }
    path(bool closed, const std::vector< point >& p)
        : _closed(closed), _p(p)
    {
    }
    
    path(const char* str);
    operator std::string() const;
    
    bool closed() const { return _closed;}
    bool& closed() { return _closed;}
    void closed(bool val){ _closed = val;}
    
    std::vector< point > points() const { return _p;}
    std::vector< point >& points() { return _p;}
    
private:
    bool _closed;
    std::vector< point > _p;
};

class polygon {
    friend pq_async::polygon pgval_to_polygon(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::polygon& value);
    
    friend std::ostream& operator<<(std::ostream& os, const polygon& v);
    friend std::istream& operator>> (std::istream& is, polygon& v);
    
public:
    polygon()
    {
    }
    polygon(const box& b, const std::vector< point >& p)
        : _boundbox(b), _p(p)
    {
    }

    polygon(const char* str);
    operator std::string() const;

    box bound_box() const { return _boundbox;}
    box& bound_box() { return _boundbox;}
    void bound_box(const box& val){ _boundbox = val;}
    
    std::vector< point > points() const { return _p;}
    std::vector< point >& points() { return _p;}
    
private:
    box _boundbox;
    std::vector< point > _p;
};

class circle {
    friend pq_async::circle pgval_to_circle(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::circle& value);
    
    friend std::ostream& operator<<(std::ostream& os, const circle& v);
    friend std::istream& operator>> (std::istream& is, circle& v);
    
public:
    circle()
    {
    }
    circle(point c, double r)
        : _center(c), _radius(r)
    {
    }
    
    circle(const char* str);
    operator std::string() const;
    
    point center() const { return _center;}
    point& center() { return _center;}
    void center(const point& val){ _center = val;}
    
    double radius() const { return _radius;}
    double& radius() { return _radius;}
    void radius(double val){ _radius = val;}
    
private:
    point _center;
    double _radius;
};


std::ostream& operator<<(std::ostream& os, const point& v);
std::istream& operator>> (std::istream& is, point& v);
std::ostream& operator<<(std::ostream& os, const line& v);
std::istream& operator>> (std::istream& is, line& v);
std::ostream& operator<<(std::ostream& os, const lseg& v);
std::istream& operator>> (std::istream& is, lseg& v);
std::ostream& operator<<(std::ostream& os, const box& v);
std::istream& operator>> (std::istream& is, box& v);
std::ostream& operator<<(std::ostream& os, const path& v);
std::istream& operator>> (std::istream& is, path& v);
std::ostream& operator<<(std::ostream& os, const polygon& v);
std::istream& operator>> (std::istream& is, polygon& v);
std::ostream& operator<<(std::ostream& os, const circle& v);
std::istream& operator>> (std::istream& is, circle& v);

} // ns: pq_async

#endif // _pq_asyncpp_geo_h