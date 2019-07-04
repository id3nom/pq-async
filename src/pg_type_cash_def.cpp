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

#include "pg_type_cash_def.h"

#include <locale>

namespace pq_async{

std::locale money::_default_locale = std::locale("");

void money::decimal_parts_to(
    bool& is_positive, std::string& whole_part, std::string& decim_part) const
{
    int64_t m = std::pow(10L, _frac_digits);
    int64_t a = std::abs(_val) / m;
    int64_t b = std::abs(_val) % m;
    is_positive = _val >= 0;
    whole_part = md::num_to_str(a, false);
    decim_part = md::num_to_str(b, false);
}

void money::decimal_parts_to(
    int64_t& a, int64_t& b) const
{
    int64_t m = std::pow(10L, _frac_digits);
    a = _val / m;
    b = std::abs(_val) % m;
    if(_val < 0)
        b *= -1;
}

void money::to_frac_digits(int64_t frac_digits)
{
    if(frac_digits == _frac_digits)
        return;
    
    if(frac_digits > _frac_digits){
        int64_t m = std::pow(10L, frac_digits - _frac_digits);
        _val *= m;
        _frac_digits = frac_digits;
        
    }else{
        int64_t m = std::pow(10L, _frac_digits - frac_digits);
        _val /= m;
        _frac_digits = frac_digits;
        
    }
}

money money::to_frac_digits(const money& m, int64_t frac_digits)
{
    money tm(m);
    tm.to_frac_digits(frac_digits);
    return tm;
}


std::ostream& operator<<(std::ostream& os, const money& v)
{
    os << (std::string)v;
    return os;
}





} // ns: pq_async