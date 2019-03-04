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

#ifndef _pq_asyncpp_uuid_h
#define _pq_asyncpp_uuid_h

#include "exceptions.h"
#include "utils.h"


namespace pq_async {

class uuid {
public:
    uuid()
        : _uuid{
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
        }
    {
    }
    
    uuid(const uint8_t* val, size_t max_len = 16)
        : _uuid{}
    {
        if(max_len > 16)
            max_len = 16;
        std::copy(val, val +max_len, _uuid);
    }
    
    uuid(const uuid& b)
    {
        for(size_t i = 0; i < this->size(); ++i)
            _uuid[i] = b[i];
    }
    
    size_t size() const { return 16;}
    
    uint8_t& operator [](size_t idx)
    {
        if(idx > 16)
            throw pq_async::exception("index is out of bound!");
        return _uuid[idx];
    }
    
    uint8_t operator [](size_t idx) const
    {
        if(idx > 16)
            throw pq_async::exception("index is out of bound!");
        return _uuid[idx];
    }
    
    const uint8_t* data() const
    {
        return &_uuid[0];
    }
    
    operator std::string() const
    {
        return hex_to_str((uint8_t*)_uuid, 4) + "-" +
            hex_to_str((uint8_t*)(_uuid +4), 2) + "-" +
            hex_to_str((uint8_t*)(_uuid +6), 2) + "-" +
            hex_to_str((uint8_t*)(_uuid +8), 2) + "-" +
            hex_to_str((uint8_t*)(_uuid +10), 6);
    }
    
    bool operator ==(const uuid& b) const
    {
        for(size_t i = 0; i < this->size(); ++i)
            if(_uuid[i] != b[i])
                return false;
        return true;
    }
    
    bool operator !=(const uuid& b) const
    {
        return !(*this == b);
    }
    
    bool operator <(const uuid& b) const
    {
        for(size_t i = 0; i < this->size(); ++i)
            if(_uuid[i] > b[i])
                return false;
            else if(_uuid[i] < b[i])
                return true;
        return false;
    }
    
    bool operator >(const uuid& b) const
    {
        return !(*this < b);
    }
    
    bool operator <=(const uuid& b) const
    {
        return (*this < b) || (*this == b);
    }
    
    bool operator >=(const uuid& b) const
    {
        return (*this > b) || (*this == b);
    }
    
    static uuid generate();
    
private:
    uint8_t _uuid[16];
    
};

} //ns pq_async

#endif //_pq_asyncpp_uuid_h