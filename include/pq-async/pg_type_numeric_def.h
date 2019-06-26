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

#ifndef _pq_asyncpp_numeric_h
#define _pq_asyncpp_numeric_h

#include "exceptions.h"
#include "utils.h"

namespace pq_async {


class numeric;
// the partial specialization of range is enabled via a template parameter
template<class T, class Enable = void>
class range {}; // primary template
typedef range<numeric> numrange;
class parameter;

class numeric {
    friend pq_async::numeric pgval_to_numeric(char* val, int len, int fmt);
    friend pq_async::parameter* new_parameter(const pq_async::numeric& value);
    friend pq_async::parameter* new_parameter(const pq_async::numrange& value);

    friend void copy_var_to_var(const numeric& src, numeric& dest);
    friend void alloc_var(numeric *var, int ndigits);
    friend void free_var(numeric *var);
    friend void zero_var(numeric *var);
    friend const char* set_var_from_str(
        const char *str, const char *cp, numeric *dest);
    friend char* get_str_from_var(const numeric* var);
    friend char* get_str_from_var_sci(const numeric* var, int rscale);
    
    friend bool numericvar_to_int64(const numeric *var, int64_t *result);
    friend void int64_to_numericvar(int64_t val, numeric *var);
    friend int	cmp_var(const numeric *var1, const numeric *var2);
    
    friend void add_var(const numeric* var1, const numeric *var2,
        numeric *result);
    friend void sub_var(const numeric *var1, const numeric *var2,
        numeric *result);
    friend void mul_var(const numeric *var1, const numeric *var2,
        numeric *result,
        int rscale);
    friend void div_var(const numeric *var1, const numeric *var2,
        numeric *result,
        int rscale, bool round);
    friend void div_var_fast(const numeric *var1, const numeric *var2,
        numeric *result, int rscale, bool round);
    friend int	select_div_scale(
        const numeric *var1, const numeric *var2);
    friend void mod_var(const numeric *var1, const numeric *var2,
        numeric *result);
    friend void ceil_var(const numeric *var, numeric *result);
    friend void floor_var(const numeric *var, numeric *result);
    
    friend void sqrt_var(const numeric *arg, numeric *result, int rscale);
    friend void exp_var(const numeric *arg, numeric *result, int rscale);
    friend int	estimate_ln_dweight(const numeric *var);
    friend void ln_var(const numeric *arg, numeric *result, int rscale);
    friend void log_var(const numeric *base, const numeric *num,
        numeric *result);
    friend void power_var(const numeric *base, const numeric *exp,
        numeric *result);
    friend void power_var_int(const numeric *base, int exp, numeric *result,
        int rscale);
    
    friend int	cmp_abs(const numeric *var1, const numeric *var2);

    friend void add_abs(const numeric *var1, const numeric *var2,
        numeric *result);
    friend void sub_abs(const numeric *var1, const numeric *var2,
        numeric *result);
    friend void round_var(numeric *var, int rscale);
    friend void trunc_var(numeric *var, int rscale);
    friend void strip_var(numeric *var);
    
    friend std::ostream& operator<<(std::ostream& s, const numeric& v);
public:
    numeric();
    numeric(const numeric& b);
    numeric(const char* s, ssize_t len = -1);
    numeric(double val);
    numeric(float val);
    numeric(int64_t val);
    numeric(int32_t val);
    ~numeric();
    
    operator std::string() const;
    void decimal_parts_to(
        bool& is_positive, std::string& whole_part, std::string& dec_part
    ) const;
    
    
    bool is_nan() const;
    
    bool operator ==(const numeric& b) const;
    bool operator !=(const numeric& b) const;
    bool operator <(const numeric& b) const;
    bool operator >(const numeric& b) const;
    bool operator <=(const numeric& b) const;
    bool operator >=(const numeric& b) const;
    numeric& operator=(const numeric& b);
    numeric operator+(const numeric& b) const;
    numeric operator-(const numeric& b) const;
    numeric operator*(const numeric& b) const;
    numeric operator/(const numeric& b) const;
    numeric operator%(const numeric& b) const;
    numeric& operator+=(const numeric& b);
    numeric& operator-=(const numeric& b);
    numeric& operator*=(const numeric& b);
    numeric& operator/=(const numeric& b);
    numeric& operator%=(const numeric& b);
    numeric& operator++(); // prefix
    numeric operator++(int /*unused*/); // postfix
    numeric& operator--(); // prefix
    numeric operator--(int /*unused*/); // postfix
    // operator decimal64() const;
    // decimal64 to_dec64() const;
    operator int64_t() const;
    operator int32_t() const;
    operator double() const;
    
    static numeric nan();
    static numeric from(int64_t val);
    static numeric from(int32_t val);
    static numeric from(double val);
    
    static int cmp(const numeric& var1, const numeric& var2);
    static void add(const numeric& var1, const numeric& var2, numeric& result);
    
private:
    int32_t ndigits;	/* # of digits in digits[] - can be 0! */
    int32_t weight;		/* weight of first digit */
    int32_t sign;		/* NUMERIC_POS, NUMERIC_NEG, or NUMERIC_NAN */
    int32_t dscale;		/* display scale */
    int16_t* buf;		/* start of palloc'd space for digits[] */
    int16_t* digits;	/* base-NBASE digits */
};

std::ostream& operator<<(std::ostream& os, const numeric& v);

} //ns pq_async


#endif //_pq_asyncpp_numeric_h