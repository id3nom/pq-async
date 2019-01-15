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

#ifndef _pq_asyncpp_net_h
#define _pq_asyncpp_net_h

#include <sys/socket.h>
#include <cstdio>

#include "exceptions.h"
#include "utils.h"

namespace pq_async {

class parameter;

enum class net_family{
    pq_net_family_none = PF_UNSPEC,
    pq_net_family_inet = PF_INET,
    pq_net_family_inet6 = PF_INET6,
};

class cidr {
    friend pq_async::cidr pgval_to_cidr(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::cidr& value);

    friend std::ostream& operator<<(std::ostream& s, const cidr& v);
    friend std::istream& operator>> (std::istream& is, cidr& v);

public:
    
    cidr()
        : _family(net_family::pq_net_family_none), _bits(0), _ipaddr{}
    {
    }
    
    cidr(const cidr& b)
    {
        this->_family = b._family;
        this->_bits = b._bits;
        std::copy(b._ipaddr, b._ipaddr +16, this->_ipaddr);
    }
    
    cidr& operator=(const cidr& b)
    {
        this->_family = b._family;
        this->_bits = b._bits;
        std::copy(b._ipaddr, b._ipaddr +16, this->_ipaddr);

        return *this;
    }
    
    cidr(const char* str);
    operator std::string() const;
    
    
    inline net_family get_family() const
    {
        return _family;
    }
    inline void set_family(net_family family)
    {
        _family = family;
    }
    inline uint8_t get_mask_bits() const
    {
        return _bits;
    }
    inline void set_mask_bits(uint8_t bits)
    {
        _bits = bits;
    }
    
    size_t size() const
    {
        return _family == net_family::pq_net_family_inet ? 4 : 16;
    }
    
    uint8_t& operator [](size_t idx)
    {
        if(idx > this->size())
            throw pq_async::exception("index is out of bound!");
        return _ipaddr[idx];
    }
    
    uint8_t operator [](size_t idx) const
    {
        if(idx > this->size())
            throw pq_async::exception("index is out of bound!");
        return _ipaddr[idx];
    }
    
    const uint8_t* data() const
    {
        return &_ipaddr[0];
    }
    
    bool operator ==(const cidr& b) const;
    bool operator !=(const cidr& b) const;
    bool operator <(const cidr& b) const;
    bool operator >(const cidr& b) const;
    bool operator <=(const cidr& b) const;
    bool operator >=(const cidr& b) const;

    
private:
    net_family _family;
    uint8_t _bits;
    uint8_t _ipaddr[16];
};

class inet {
    friend pq_async::inet pgval_to_inet(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::inet& value);

    friend std::ostream& operator<<(std::ostream& s, const inet& v);
    friend std::istream& operator>> (std::istream& is, inet& v);
    
public:
    inet()
        : _family(net_family::pq_net_family_none), _bits(0), _ipaddr{}
    {
    }
    
    inet(const inet& b)
    {
        this->_family = b._family;
        this->_bits = b._bits;
        std::copy(b._ipaddr, b._ipaddr +16, this->_ipaddr);
    }
    
    inet& operator=(const inet& b)
    {
        this->_family = b._family;
        this->_bits = b._bits;
        std::copy(b._ipaddr, b._ipaddr +16, this->_ipaddr);

        return *this;
    }
    
    inet(const char* str);
    operator std::string() const;
    
    inline net_family get_family() const
    {
        return _family;
    }
    inline void set_family(net_family family)
    {
        _family = family;
    }
    inline uint8_t get_mask_bits() const
    {
        return _bits;
    }
    inline void set_mask_bits(uint8_t bits)
    {
        _bits = bits;
    }
    
    size_t size() const 
    {
        return _family == net_family::pq_net_family_inet ? 4 : 16;
    }
    
    uint8_t& operator [](size_t idx)
    {
        if(idx > this->size())
            throw pq_async::exception("index is out of bound!");
        return _ipaddr[idx];
    }
    
    uint8_t operator [](size_t idx) const
    {
        if(idx > this->size())
            throw pq_async::exception("index is out of bound!");
        return _ipaddr[idx];
    }
    
    const uint8_t* data() const
    {
        return &_ipaddr[0];
    }
    
    bool operator ==(const inet& b) const;
    bool operator !=(const inet& b) const;
    bool operator <(const inet& b) const;
    bool operator >(const inet& b) const;
    bool operator <=(const inet& b) const;
    bool operator >=(const inet& b) const;
    
    
private:
    net_family _family;
    uint8_t _bits;
    uint8_t _ipaddr[16];
};

class macaddr {
    friend pq_async::macaddr pgval_to_macaddr(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::macaddr& value);

    friend std::ostream& operator<<(std::ostream& s, const macaddr& v);
    friend std::istream& operator>> (std::istream& is, macaddr& v);
public:
    macaddr()
        : a(0), b(0), c(0), d(0), e(0), f(0)
    {
    }
    
    macaddr(uint8_t _a, uint8_t _b, uint8_t _c,
        uint8_t _d, uint8_t _e, uint8_t _f)
        : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f)
    {
    }
    
    macaddr(const char* str);
    
    operator std::string() const
    {
        char result[32] = {};
        snprintf(
            result, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
             this->a, this->b, this->c, this->d, this->e, this->f
        );
        return std::string(result);
    }
    
    
    bool operator ==(const macaddr& b) const;
    bool operator !=(const macaddr& b) const;
    bool operator <(const macaddr& b) const;
    bool operator >(const macaddr& b) const;
    bool operator <=(const macaddr& b) const;
    bool operator >=(const macaddr& b) const;
    
    
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
};

class macaddr8 {
    friend pq_async::macaddr8 pgval_to_macaddr8(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::macaddr8& value);

    friend std::ostream& operator<<(std::ostream& s, const macaddr8& v);
    friend std::istream& operator>> (std::istream& is, macaddr8& v);
    
public:
    macaddr8()
        : a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0)
    {
    }
    
    macaddr8(uint8_t _a, uint8_t _b, uint8_t _c, uint8_t _d, 
        uint8_t _e, uint8_t _f, uint8_t _g, uint8_t _h)
        : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f), g(_g), h(_h)
    {
    }
    
    macaddr8(const char* str);
    
    operator std::string() const
    {
        char result[32] = {};
        snprintf(
            result, 32, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
            this->a, this->b, this->c, this->d,
            this->e, this->f, this->g, this->h
        );
        return std::string(result);
    }
    
    bool operator ==(const macaddr8& b) const;
    bool operator !=(const macaddr8& b) const;
    bool operator <(const macaddr8& b) const;
    bool operator >(const macaddr8& b) const;
    bool operator <=(const macaddr8& b) const;
    bool operator >=(const macaddr8& b) const;
    

    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
    uint8_t g;
    uint8_t h;
};


std::ostream& operator<<(std::ostream& os, const cidr& v);
std::ostream& operator<<(std::ostream& os, const inet& v);
std::ostream& operator<<(std::ostream& os, const macaddr& v);
std::ostream& operator<<(std::ostream& os, const macaddr8& v);

std::istream& operator>> (std::istream& is, cidr& v);
std::istream& operator>> (std::istream& is, inet& v);
std::istream& operator>> (std::istream& is, macaddr& v);
std::istream& operator>> (std::istream& is, macaddr8& v);


} // ns: pq_async

#endif // _pq_asyncpp_net_h