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

#ifndef _pq_asyncpp_sys_h
#define _pq_asyncpp_sys_h

#include "exceptions.h"
#include "utils.h"


namespace pq_async {

class oid {
public:
    oid()
        : _oid(0)
    {
    }
    
    oid(const u_int32_t val)
        : _oid(val)
    {
    }
    
    oid(const oid& b)
    {
        _oid = b._oid;
    }
    
    u_int32_t data() const
    {
        return _oid;
    }
    
    operator u_int32_t() const
    {
        return _oid;
    }
    
    operator std::string() const
    {
        return md::num_to_str(_oid);
    }
    
    bool operator ==(const oid& b) const
    {
        return _oid == b._oid;
    }
    
    bool operator !=(const oid& b) const
    {
        return _oid != b._oid;
    }
    
    bool operator <(const oid& b) const
    {
        return _oid < b._oid;
    }
    
    bool operator >(const oid& b) const
    {
        return _oid > b._oid;
    }
    
    bool operator <=(const oid& b) const
    {
        return _oid <= b._oid;
    }
    
    bool operator >=(const oid& b) const
    {
        return _oid >= b._oid;
    }
    
private:
    u_int32_t _oid;
    
};

} //ns pq_async

#endif //_pq_asyncpp_sys_h