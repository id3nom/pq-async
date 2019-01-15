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

#include "pg_type_numeric_def.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <limits.h>

#if __GNUC__ >= 3
#define likely(x)	__builtin_expect((x) != 0, 1)
#define unlikely(x) __builtin_expect((x) != 0, 0)
#else
#define likely(x)	((x) != 0)
#define unlikely(x) ((x) != 0)
#endif


/* msb for char */
#define HIGHBIT					(0x80)
#define IS_HIGHBIT_SET(ch)		((unsigned char)(ch) & HIGHBIT)

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#define INT64CONST(x)  (x##LL)
#define UINT64CONST(x) (x##ULL)

/*
 * stdint.h limits aren't guaranteed to be present and aren't guaranteed to
 * have compatible types with our fixed width types. So just define our own.
 */
#define PG_INT8_MIN		(-0x7F-1)
#define PG_INT8_MAX		(0x7F)
#define PG_UINT8_MAX	(0xFF)
#define PG_INT16_MIN	(-0x7FFF-1)
#define PG_INT16_MAX	(0x7FFF)
#define PG_UINT16_MAX	(0xFFFF)
#define PG_INT32_MIN	(-0x7FFFFFFF-1)
#define PG_INT32_MAX	(0x7FFFFFFF)
#define PG_UINT32_MAX	(0xFFFFFFFFU)
#define PG_INT64_MIN	(-INT64CONST(0x7FFFFFFFFFFFFFFF) - 1)
#define PG_INT64_MAX	INT64CONST(0x7FFFFFFFFFFFFFFF)
#define PG_UINT64_MAX	UINT64CONST(0xFFFFFFFFFFFFFFFF)



#if 1
#define NBASE		10000
#define HALF_NBASE	5000
#define DEC_DIGITS	4			/* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS	2	/* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS	4

typedef int16_t NumericDigit;
#endif

#define NUMERIC_SIGN_MASK	0xC000
#define NUMERIC_POS			0x0000
#define NUMERIC_NEG			0x4000
#define NUMERIC_SHORT		0x8000
#define NUMERIC_NAN			0xC000
#define NUMERIC_MAX_PRECISION			1000
#define NUMERIC_MAX_DISPLAY_SCALE		NUMERIC_MAX_PRECISION
#define NUMERIC_MIN_DISPLAY_SCALE		0
#define NUMERIC_MIN_SIG_DIGITS			16
#define NUMERIC_MAX_RESULT_SCALE	(NUMERIC_MAX_PRECISION * 2)


/*
#define digitbuf_alloc(ndigits)  \
    ((NumericDigit *) malloc((ndigits) * sizeof(NumericDigit)))
#define digitbuf_free(buf)	\
    do { \
         if ((buf) != NULL) \
             free(buf); \
    } while (0)
*/
#define digitbuf_alloc(ndigits)  \
    ((NumericDigit *) new NumericDigit[ndigits])
#define digitbuf_free(buf)	\
    do { \
         if ((buf) != NULL) \
             delete[] buf; \
    } while (0)


namespace pq_async{
    
    static const numeric const_zero("0");
    static const numeric const_one_point_one("0.1");
    static const numeric const_zero_point_five("0.5");
    static const numeric const_zero_point_nine("0.9");
    static const numeric const_one("1");
    static const numeric const_two("2");
    static const numeric const_ten("10");
    
    #if DEC_DIGITS == 4
    static const int round_powers[4] = {0, 1000, 100, 10};
    #endif

    
    int pg_strncasecmp(const char *s1, const char *s2, size_t n);
    
    void alloc_var(numeric *var, int ndigits);
    void free_var(numeric *var);
    void zero_var(numeric *var);
    
    const char* set_var_from_str(const char* str, const char* cp,
        numeric* dest);
    char* get_str_from_var(const numeric* var);
    char* get_str_from_var_sci(const numeric* var, int rscale);
    
    int32_t numericvar_to_int32(const numeric* var);
    bool numericvar_to_int64(const numeric* var, int64_t* result);
    void int64_to_numericvar(int64_t val, numeric* var);
    double numericvar_to_double_no_overflow(const numeric *var);
    
    int	cmp_var(const numeric *var1, const numeric *var2);
    int cmp_var_common(
        const NumericDigit *var1digits, int var1ndigits,
        int var1weight, int var1sign,
        const NumericDigit *var2digits, int var2ndigits,
        int var2weight, int var2sign
    );
    void add_var(const numeric* var1, const numeric *var2,
        numeric *result);
    void sub_var(const numeric *var1, const numeric *var2,
        numeric *result);
    void mul_var(const numeric *var1, const numeric *var2,
        numeric *result,
        int rscale);
    void div_var(const numeric *var1, const numeric *var2,
        numeric *result,
        int rscale, bool round);
    void div_var_fast(const numeric *var1, const numeric *var2,
        numeric *result, int rscale, bool round);
    int	select_div_scale(
        const numeric *var1, const numeric *var2);
    void mod_var(const numeric *var1, const numeric *var2,
        numeric *result);
    void ceil_var(const numeric *var, numeric *result);
    void floor_var(const numeric *var, numeric *result);

    void sqrt_var(const numeric *arg, numeric *result, int rscale);
    void exp_var(const numeric *arg, numeric *result, int rscale);
    int	estimate_ln_dweight(const numeric *var);
    void ln_var(const numeric *arg, numeric *result, int rscale);
    void log_var(const numeric *base, const numeric *num,
        numeric *result);
    void power_var(const numeric *base, const numeric *exp,
        numeric *result);
    void power_var_int(const numeric *base, int exp, numeric *result,
        int rscale);
    
    int	cmp_abs(const numeric *var1, const numeric *var2);
    int cmp_abs_common(const NumericDigit *var1digits, int var1ndigits,
        int var1weight,
        const NumericDigit *var2digits, int var2ndigits,
        int var2weight
    );
    void add_abs(const numeric *var1, const numeric *var2,
        numeric *result);
    void sub_abs(const numeric *var1, const numeric *var2,
        numeric *result);
    void round_var(numeric *var, int rscale);
    void trunc_var(numeric *var, int rscale);
    void strip_var(numeric *var);
    
    void copy_var_to_var(const numeric& src, numeric& dest)
    {
        dest.ndigits = src.ndigits;
        dest.weight = src.weight;
        dest.sign = src.sign;
        dest.dscale = src.dscale;
        
        NumericDigit *newbuf;
        
        newbuf = digitbuf_alloc(src.ndigits + 1);
        newbuf[0] = 0;				/* spare digit for rounding */
        if(src.ndigits > 0)		/* else value->digits might be null */
            memcpy(newbuf + 1, src.digits, src.ndigits * sizeof(NumericDigit));
        
        digitbuf_free(dest.buf);
        dest.buf = newbuf;
        dest.digits = newbuf + 1;
    }
    
    numeric::numeric()
        : ndigits(0), weight(0), sign(NUMERIC_POS), dscale(0),
        buf(nullptr), digits(nullptr)
    {
    }
    
    numeric::numeric(const numeric& b)
        : ndigits(0), weight(0), sign(NUMERIC_POS), dscale(0),
        buf(nullptr), digits(nullptr)
    {
        this->ndigits = b.ndigits;
        this->weight = b.weight;
        this->sign = b.sign;
        this->dscale = b.dscale;
        
        NumericDigit *newbuf = digitbuf_alloc(this->ndigits + 1);
        newbuf[0] = 0;				/* spare digit for rounding */
        if (this->ndigits > 0)		/* else value->digits might be null */
            // for(size_t i = 0; i < (size_t)this->ndigits; ++i)
            // 	newbuf[i+1] = b.digits[i];
            memcpy(
                newbuf + 1,
                b.digits,
                b.ndigits * sizeof(NumericDigit)
            );
        
        //digitbuf_free(this->buf);
        
        this->buf = newbuf;
        this->digits = newbuf + 1;
    }
    
    numeric::numeric(const char* s, ssize_t len)
        : ndigits(0), weight(0), sign(NUMERIC_POS), dscale(0),
        buf(nullptr), digits(nullptr)
    {
        if(len < 0)
            len = strlen(s);
        
        char* str = new char[len +1];
        std::copy_n(s, len, str);
        str[len] = 0;
        
        const char* cp = nullptr;
        
        /* Skip leading spaces */
        cp = str;
        while(*cp){
            if (!isspace((unsigned char) *cp))
                break;
            cp++;
        }
        
        /*
        * Check for NaN
        */
        if(pg_strncasecmp(cp, "NaN", 3) == 0){
            this->sign = NUMERIC_NAN;
            delete[] str;
            return;
            // //res = make_result(&const_nan);
            // /* Should be nothing left but spaces */
            // cp += 3;
            // while (*cp){
            // 	if (!isspace((unsigned char) *cp))
            // 		throw pq_async::exception(
            // 			"invalid input syntax for type numeric"
            // 		);
            // 	cp++;
            // }
        } else {
            /*
            * Use set_var_from_str() to parse a normal numeric value
            */
            //numeric	value;
            //init_var(&value);
            
            try{
                cp = set_var_from_str(str, cp, this);
            }catch(...){
                this->sign = NUMERIC_NAN;
                delete[] str;
                return;
            }
            
            /*
            * We duplicate a few lines of code here because we would like to
            * throw any trailing-junk syntax error before any semantic error
            * resulting from apply_typmod.  We can't easily fold the two cases
            * together because we mustn't apply apply_typmod to a NaN.
            */
            while(*cp){
                if (!isspace((unsigned char) *cp)){
                    this->sign = NUMERIC_NAN;
                    delete[] str;
                    return;

                    //throw pq_async::exception(
                    //	"invalid input syntax for type numeric"
                    //);
                }
                cp++;
            }
            
            delete[] str;
            //apply_typmod(&value, typmod);
            
            //res = make_result(&value);
            //free_var(&value);
        }
    }
    
    numeric::numeric(double val)
        : numeric(num_to_str(val).c_str())
    {
    }
    numeric::numeric(float val)
        : numeric(num_to_str(val).c_str())
    {
    }
    numeric::numeric(int64_t val)
        : numeric(num_to_str(val).c_str())
    {
    }
    numeric::numeric(int32_t val)
        : numeric(num_to_str(val).c_str())
    {
    }
    
    numeric::~numeric()
    {
        digitbuf_free(this->buf);
    }
    
    numeric::operator std::string() const
    {
        char* s = get_str_from_var(this);
        std::string str(s);
        delete[] s;
        return str;
    }
    
    bool numeric::is_nan() const
    {
        return sign == NUMERIC_NAN;
    }
    
    numeric& numeric::operator=(const numeric& b)
    {
        this->ndigits = b.ndigits;
        this->weight = b.weight;
        this->sign = b.sign;
        this->dscale = b.dscale;
        
        NumericDigit *newbuf = digitbuf_alloc(this->ndigits + 1);
        newbuf[0] = 0;				/* spare digit for rounding */
        if (this->ndigits > 0)		/* else value->digits might be null */
            // for(size_t i = 0; i < (size_t)this->ndigits; ++i)
            // 	newbuf[i+1] = b.digits[i];
            memcpy(
                newbuf + 1,
                b.digits,
                b.ndigits * sizeof(NumericDigit)
            );
        
        digitbuf_free(this->buf);
        
        this->buf = newbuf;
        this->digits = newbuf + 1;
        
        return *this;
    }
    
    bool numeric::operator ==(const numeric& b) const
    {
        return numeric::cmp(*this, b) == 0;
    }

    bool numeric::operator !=(const numeric& b) const
    {
        return numeric::cmp(*this, b) != 0;
    }

    bool numeric::operator <(const numeric& b) const
    {
        return numeric::cmp(*this, b) < 0;
    }

    bool numeric::operator >(const numeric& b) const
    {
        return numeric::cmp(*this, b) > 0;
    }

    bool numeric::operator <=(const numeric& b) const
    {
        return numeric::cmp(*this, b) <= 0;
    }

    bool numeric::operator >=(const numeric& b) const
    {
        return numeric::cmp(*this, b) >= 0;
    }
    
    numeric numeric::operator+(const numeric& b) const
    {
        numeric result;
        add_var(this, &b, &result);
        return result;
    }
    
    numeric numeric::operator-(const numeric& b) const
    {
        numeric result;
        sub_var(this, &b, &result);
        return result;
    }
    
    numeric numeric::operator*(const numeric& b) const
    {
        numeric result;
        mul_var(this, &b, &result, std::max(this->dscale, b.dscale));
        return result;
    }
    
    numeric numeric::operator/(const numeric& b) const
    {
        numeric result;
        div_var(this, &b, &result, std::max(this->dscale, b.dscale), false);
        return result;
    }
    
    numeric numeric::operator%(const numeric& b) const
    {
        numeric result;
        mod_var(this, &b, &result);
        return result;
    }
    
    numeric& numeric::operator+=(const numeric& b)
    {
        numeric tmp;
        add_var(this, &b, &tmp);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric& numeric::operator-=(const numeric& b)
    {
        numeric tmp;
        sub_var(this, &b, &tmp);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric& numeric::operator*=(const numeric& b)
    {
        numeric tmp;
        mul_var(this, &b, &tmp, std::max(this->dscale, b.dscale));
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric& numeric::operator/=(const numeric& b)
    {
        numeric tmp;
        div_var(this, &b, &tmp, std::max(this->dscale, b.dscale), false);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric& numeric::operator%=(const numeric& b)
    {
        numeric tmp;
        mod_var(this, &b, &tmp);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric& numeric::operator++() // prefix
    {
        numeric tmp;
        add_var(this, &const_one, &tmp);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric numeric::operator++(int /*unused*/) // postfix
    {
        numeric res = *this;
        ++(*this);
        return res;
        
        //SomeValue result = *this;
        //++(*this); // call SomeValue::operator++()
        //return result;
        //throw pq_async::exception("NOT IMPLEMENTED!");
    }

    numeric& numeric::operator--() // prefix
    {
        numeric tmp;
        sub_var(this, &const_one, &tmp);
        copy_var_to_var(tmp, *this);
        return *this;
    }
    
    numeric numeric::operator--(int /*unused*/) // postfix
    {
        numeric res = *this;
        --(*this);
        return res;
    }

    
    // numeric::operator decimal64() const
    // {
    // 	return to_dec64();
    // }
    // decimal64 numeric::to_dec64() const
    // {
    // 	// decContext set;
    // 	// decContextDefault(&set, DEC_INIT_DECIMAL64);
    // 	// decimal64 dec;
        
    // 	throw pq_async::exception("NOT IMPLEMENTED!");
    // }
    
    numeric::operator int64_t() const
    {
        int64_t result;
        if(numericvar_to_int64(this, &result))
            return result;
        throw pq_async::exception("Unable to convert to int64!");
    }
    
    numeric::operator int32_t() const
    {
        return numericvar_to_int32(this);
    }
    
    numeric::operator double() const
    {
        return numericvar_to_double_no_overflow(this);
    }

    int numeric::cmp(const numeric& var1, const numeric& var2)
    {
        return cmp_var_common(
            var1.digits, var1.ndigits,
            var1.weight, var1.sign,
            var2.digits, var2.ndigits,
            var2.weight, var2.sign
        );
    }
    
    numeric numeric::nan()
    {
        numeric n;
        n.sign = NUMERIC_NAN;
        return n;
    }
    
    numeric numeric::from(int64_t val)
    {
        numeric result;
        int64_to_numericvar(val, &result);
        return result;
    }
    numeric numeric::from(int32_t val)
    {
        return numeric::from((int64_t)val);
    }
    numeric numeric::from(double val)
    {
        return numeric(num_to_str(val).c_str());
    }
    
    std::ostream& operator<<(std::ostream& os, const numeric& v)
    {
        os << (std::string)v;
        return os;
    }
    
    
    /*
    * Case-independent comparison of two not-necessarily-null-terminated strings.
    * At most n bytes will be examined from each string.
    */
    int pg_strncasecmp(const char *s1, const char *s2, size_t n)
    {
        while (n-- > 0){
            unsigned char ch1 = (unsigned char) *s1++;
            unsigned char ch2 = (unsigned char) *s2++;

            if (ch1 != ch2){
                if (ch1 >= 'A' && ch1 <= 'Z')
                    ch1 += 'a' - 'A';
                else if (IS_HIGHBIT_SET(ch1) && isupper(ch1))
                    ch1 = tolower(ch1);

                if (ch2 >= 'A' && ch2 <= 'Z')
                    ch2 += 'a' - 'A';
                else if (IS_HIGHBIT_SET(ch2) && isupper(ch2))
                    ch2 = tolower(ch2);

                if (ch1 != ch2)
                    return (int) ch1 - (int) ch2;
            }
            if (ch1 == 0)
                break;
        }
        return 0;
    }
    
    
    void alloc_var(numeric *var, int ndigits)
    {
        digitbuf_free(var->buf);
        var->buf = digitbuf_alloc(ndigits + 1);
        var->buf[0] = 0;			/* spare digit for rounding */
        var->digits = var->buf + 1;
        var->ndigits = ndigits;
    }


    /*
    * free_var() -
    *
    *	Return the digit buffer of a variable to the free pool
    */
    void free_var(numeric *var)
    {
        digitbuf_free(var->buf);
        var->buf = NULL;
        var->digits = NULL;
        var->sign = NUMERIC_NAN;
    }

    /*
    * zero_var() -
    *
    *	Set a variable to ZERO.
    *	Note: its dscale is not touched.
    */
    void zero_var(numeric *var)
    {
        digitbuf_free(var->buf);
        var->buf = NULL;
        var->digits = NULL;
        var->ndigits = 0;
        var->weight = 0;			/* by convention; doesn't really matter */
        var->sign = NUMERIC_POS;	/* anything but NAN... */
    }
    
    
    
    
/*
 * set_var_from_str()
 *
 *	Parse a string and put the number into a variable
 *
 * This function does not handle leading or trailing spaces, and it doesn't
 * accept "NaN" either.  It returns the end+1 position so that caller can
 * check for trailing spaces/garbage if deemed necessary.
 *
 * cp is the place to actually start parsing; str is what to use in error
 * reports.  (Typically cp would be the same except advanced over spaces.)
 */
const char*
set_var_from_str(const char *str, const char *cp, numeric *dest)
{
    bool		have_dp = false;
    int			i;
    unsigned char *decdigits;
    int			sign = NUMERIC_POS;
    int			dweight = -1;
    int			ddigits;
    int			dscale = 0;
    int			weight;
    int			ndigits;
    int			offset;
    NumericDigit *digits;

    /*
     * We first parse the string to extract decimal digits and determine the
     * correct decimal weight.  Then convert to NBASE representation.
     */
    switch (*cp)
    {
        case '+':
            sign = NUMERIC_POS;
            cp++;
            break;

        case '-':
            sign = NUMERIC_NEG;
            cp++;
            break;
    }

    if (*cp == '.')
    {
        have_dp = true;
        cp++;
    }
    
    if (!isdigit((unsigned char) *cp))
        throw pq_async::exception(
            "invalid input syntax for type numeric"
        );
    
    decdigits = (unsigned char *)malloc(strlen(cp) + DEC_DIGITS * 2);

    /* leading padding for digit alignment later */
    memset(decdigits, 0, DEC_DIGITS);
    i = DEC_DIGITS;

    while (*cp)
    {
        if (isdigit((unsigned char) *cp))
        {
            decdigits[i++] = *cp++ - '0';
            if (!have_dp)
                dweight++;
            else
                dscale++;
        }
        else if (*cp == '.')
        {
            if(have_dp)
                throw pq_async::exception(
                    "invalid input syntax for type numeric"
                );
            
            have_dp = true;
            cp++;
        }
        else
            break;
    }

    ddigits = i - DEC_DIGITS;
    /* trailing padding for digit alignment later */
    memset(decdigits + i, 0, DEC_DIGITS - 1);

    /* Handle exponent, if any */
    if (*cp == 'e' || *cp == 'E')
    {
        long		exponent;
        char	   *endptr;

        cp++;
        exponent = strtol(cp, &endptr, 10);
        if (endptr == cp)
            throw pq_async::exception(
                "invalid input syntax for type numeric"
            );
        cp = endptr;

        /*
         * At this point, dweight and dscale can't be more than about
         * INT_MAX/2 due to the MaxAllocSize limit on string length, so
         * constraining the exponent similarly should be enough to prevent
         * integer overflow in this function.  If the value is too large to
         * fit in storage format, make_result() will complain about it later;
         * for consistency use the same ereport errcode/text as make_result().
         */
        if (exponent >= INT_MAX / 2 || exponent <= -(INT_MAX / 2))
            throw pq_async::exception(
                "invalid input syntax for type numeric"
            );
        dweight += (int) exponent;
        dscale -= (int) exponent;
        if (dscale < 0)
            dscale = 0;
    }

    /*
     * Okay, convert pure-decimal representation to base NBASE.  First we need
     * to determine the converted weight and ndigits.  offset is the number of
     * decimal zeroes to insert before the first given digit to have a
     * correctly aligned first NBASE digit.
     */
    if (dweight >= 0)
        weight = (dweight + 1 + DEC_DIGITS - 1) / DEC_DIGITS - 1;
    else
        weight = -((-dweight - 1) / DEC_DIGITS + 1);
    offset = (weight + 1) * DEC_DIGITS - (dweight + 1);
    ndigits = (ddigits + offset + DEC_DIGITS - 1) / DEC_DIGITS;

    alloc_var(dest, ndigits);
    dest->sign = sign;
    dest->weight = weight;
    dest->dscale = dscale;

    i = DEC_DIGITS - offset;
    digits = dest->digits;

    while (ndigits-- > 0)
    {
#if DEC_DIGITS == 4
        *digits++ = ((decdigits[i] * 10 + decdigits[i + 1]) * 10 +
                     decdigits[i + 2]) * 10 + decdigits[i + 3];
#elif DEC_DIGITS == 2
        *digits++ = decdigits[i] * 10 + decdigits[i + 1];
#elif DEC_DIGITS == 1
        *digits++ = decdigits[i];
#else
#error unsupported NBASE
#endif
        i += DEC_DIGITS;
    }

    free(decdigits);

    /* Strip any leading/trailing zeroes, and normalize weight if zero */
    strip_var(dest);

    /* Return end+1 position for caller */
    return cp;
}

/*
 * get_str_from_var() -
 *
 *	Convert a var to text representation (guts of numeric_out).
 *	The var is displayed to the number of digits indicated by its dscale.
 *	Returns a palloc'd string.
 */
char *
get_str_from_var(const numeric *var)
{
    int			dscale;
    char	   *str;
    char	   *cp;
    char	   *endcp;
    int			i;
    int			d;
    NumericDigit dig;

#if DEC_DIGITS > 1
    NumericDigit d1;
#endif

    dscale = var->dscale;

    /*
     * Allocate space for the result.
     *
     * i is set to the # of decimal digits before decimal point. dscale is the
     * # of decimal digits we will print after decimal point. We may generate
     * as many as DEC_DIGITS-1 excess digits at the end, and in addition we
     * need room for sign, decimal point, null terminator.
     */
    i = (var->weight + 1) * DEC_DIGITS;
    if (i <= 0)
        i = 1;

    str = new char[i + dscale + DEC_DIGITS + 2];
    //str = (char*)malloc(i + dscale + DEC_DIGITS + 2);
    
    cp = str;

    /*
     * Output a dash for negative values
     */
    if (var->sign == NUMERIC_NEG)
        *cp++ = '-';

    /*
     * Output all digits before the decimal point
     */
    if (var->weight < 0)
    {
        d = var->weight + 1;
        *cp++ = '0';
    }
    else
    {
        for (d = 0; d <= var->weight; d++)
        {
            dig = (d < var->ndigits) ? var->digits[d] : 0;
            /* In the first digit, suppress extra leading decimal zeroes */
#if DEC_DIGITS == 4
            {
                bool		putit = (d > 0);

                d1 = dig / 1000;
                dig -= d1 * 1000;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                d1 = dig / 100;
                dig -= d1 * 100;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                d1 = dig / 10;
                dig -= d1 * 10;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                *cp++ = dig + '0';
            }
#elif DEC_DIGITS == 2
            d1 = dig / 10;
            dig -= d1 * 10;
            if (d1 > 0 || d > 0)
                *cp++ = d1 + '0';
            *cp++ = dig + '0';
#elif DEC_DIGITS == 1
            *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
        }
    }

    /*
     * If requested, output a decimal point and all the digits that follow it.
     * We initially put out a multiple of DEC_DIGITS digits, then truncate if
     * needed.
     */
    if (dscale > 0)
    {
        *cp++ = '.';
        endcp = cp + dscale;
        for (i = 0; i < dscale; d++, i += DEC_DIGITS)
        {
            dig = (d >= 0 && d < var->ndigits) ? var->digits[d] : 0;
#if DEC_DIGITS == 4
            d1 = dig / 1000;
            dig -= d1 * 1000;
            *cp++ = d1 + '0';
            d1 = dig / 100;
            dig -= d1 * 100;
            *cp++ = d1 + '0';
            d1 = dig / 10;
            dig -= d1 * 10;
            *cp++ = d1 + '0';
            *cp++ = dig + '0';
#elif DEC_DIGITS == 2
            d1 = dig / 10;
            dig -= d1 * 10;
            *cp++ = d1 + '0';
            *cp++ = dig + '0';
#elif DEC_DIGITS == 1
            *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
        }
        cp = endcp;
    }

    /*
     * terminate the string and return it
     */
    *cp = '\0';
    return str;
}

/*
 * get_str_from_var_sci() -
 *
 *	Convert a var to a normalised scientific notation text representation.
 *	This function does the heavy lifting for numeric_out_sci().
 *
 *	This notation has the general form a * 10^b, where a is known as the
 *	"significand" and b is known as the "exponent".
 *
 *	Because we can't do superscript in ASCII (and because we want to copy
 *	printf's behaviour) we display the exponent using E notation, with a
 *	minimum of two exponent digits.
 *
 *	For example, the value 1234 could be output as 1.2e+03.
 *
 *	We assume that the exponent can fit into an int32.
 *
 *	rscale is the number of decimal digits desired after the decimal point in
 *	the output, negative values will be treated as meaning zero.
 *
 *	Returns a palloc'd string.
 */
char *
get_str_from_var_sci(const numeric *var, int rscale)
{
    int32_t		exponent;
    numeric		denominator;
    numeric		significand;
    int			denom_scale;
    size_t		len;
    char	   *str;
    char	   *sig_out;
    
    if (rscale < 0)
        rscale = 0;
    
    /*
     * Determine the exponent of this number in normalised form.
     *
     * This is the exponent required to represent the number with only one
     * significant digit before the decimal place.
     */
    if (var->ndigits > 0)
    {
        exponent = (var->weight + 1) * DEC_DIGITS;

        /*
         * Compensate for leading decimal zeroes in the first numeric digit by
         * decrementing the exponent.
         */
        exponent -= DEC_DIGITS - (int) log10(var->digits[0]);
    }
    else
    {
        /*
         * If var has no digits, then it must be zero.
         *
         * Zero doesn't technically have a meaningful exponent in normalised
         * notation, but we just display the exponent as zero for consistency
         * of output.
         */
        exponent = 0;
    }

    /*
     * The denominator is set to 10 raised to the power of the exponent.
     *
     * We then divide var by the denominator to get the significand, rounding
     * to rscale decimal digits in the process.
     */
    if (exponent < 0)
        denom_scale = -exponent;
    else
        denom_scale = 0;

    //init_var(&denominator);
    //init_var(&significand);
    
    power_var_int(&const_ten, exponent, &denominator, denom_scale);
    div_var(var, &denominator, &significand, rscale, true);
    sig_out = get_str_from_var(&significand);

    //free_var(&denominator);
    //free_var(&significand);

    /*
     * Allocate space for the result.
     *
     * In addition to the significand, we need room for the exponent
     * decoration ("e"), the sign of the exponent, up to 10 digits for the
     * exponent itself, and of course the null terminator.
     */
    len = strlen(sig_out) + 13;
    str = new char[len];// (char*)malloc(len);
    snprintf(str, len, "%se%+03d", sig_out, exponent);
    
    //free(sig_out);
    delete[] sig_out;
    
    return str;
}


/*
 * Given a numeric, convert it to an int32. If the numeric
 * exceeds the range of an int32, raise the appropriate error via
 * ereport(). The input numeric is *not* free'd.
 */
int32_t
numericvar_to_int32(const numeric *var)
{
    int32_t		result;
    int64_t		val;
    
    if(!numericvar_to_int64(var, &val))
        throw pq_async::exception("integer out of range");
    
    /* Down-convert to int4 */
    result = (int32_t)val;
    
    /* Test for overflow by reverse-conversion. */
    if((int64_t) result != val)
        throw pq_async::exception("integer out of range");
    
    return result;
}


/*
 * If a - b overflows, return true, otherwise store the result of a - b into
 * *result. The content of *result is implementation defined in case of
 * overflow.
 */
inline bool
pg_sub_s64_overflow(int64_t a, int64_t b, int64_t *result)
{
#if defined(HAVE__BUILTIN_OP_OVERFLOW)
    return __builtin_sub_overflow(a, b, result);
#elif defined(HAVE_INT128)
    int128		res = (int128) a - (int128) b;

    if (res > PG_INT64_MAX || res < PG_INT64_MIN)
    {
        *result = 0x5EED;		/* to avoid spurious warnings */
        return true;
    }
    *result = (int64) res;
    return false;
#else
    if ((a < 0 && b > 0 && a < PG_INT64_MIN + b) ||
        (a > 0 && b < 0 && a > PG_INT64_MAX + b))
    {
        *result = 0x5EED;		/* to avoid spurious warnings */
        return true;
    }
    *result = a - b;
    return false;
#endif
}

/*
 * If a * b overflows, return true, otherwise store the result of a * b into
 * *result. The content of *result is implementation defined in case of
 * overflow.
 */
inline bool
pg_mul_s64_overflow(int64_t a, int64_t b, int64_t *result)
{
#if defined(HAVE__BUILTIN_OP_OVERFLOW)
    return __builtin_mul_overflow(a, b, result);
#elif defined(HAVE_INT128)
    int128		res = (int128) a * (int128) b;

    if (res > PG_INT64_MAX || res < PG_INT64_MIN)
    {
        *result = 0x5EED;		/* to avoid spurious warnings */
        return true;
    }
    *result = (int64) res;
    return false;
#else
    /*
     * Overflow can only happen if at least one value is outside the range
     * sqrt(min)..sqrt(max) so check that first as the division can be quite a
     * bit more expensive than the multiplication.
     *
     * Multiplying by 0 or 1 can't overflow of course and checking for 0
     * separately avoids any risk of dividing by 0.  Be careful about dividing
     * INT_MIN by -1 also, note reversing the a and b to ensure we're always
     * dividing it by a positive value.
     *
     */
    if ((a > PG_INT32_MAX || a < PG_INT32_MIN ||
         b > PG_INT32_MAX || b < PG_INT32_MIN) &&
        a != 0 && a != 1 && b != 0 && b != 1 &&
        ((a > 0 && b > 0 && a > PG_INT64_MAX / b) ||
         (a > 0 && b < 0 && b < PG_INT64_MIN / a) ||
         (a < 0 && b > 0 && a < PG_INT64_MIN / b) ||
         (a < 0 && b < 0 && a < PG_INT64_MAX / b)))
    {
        *result = 0x5EED;		/* to avoid spurious warnings */
        return true;
    }
    *result = a * b;
    return false;
#endif
}

/*
 * Convert numeric to int8, rounding if needed.
 *
 * If overflow, return false (no error is raised).  Return true if okay.
 */
bool
numericvar_to_int64(const numeric *var, int64_t *result)
{
    NumericDigit *digits;
    int			ndigits;
    int			weight;
    int			i;
    int64_t		val;
    bool		neg;
    numeric	rounded(*var);
    
    /* Round to nearest integer */
    //init_var(&rounded);
    //set_var_from_var(var, &rounded);
    round_var(&rounded, 0);

    /* Check for zero input */
    strip_var(&rounded);
    ndigits = rounded.ndigits;
    if (ndigits == 0)
    {
        *result = 0;
        free_var(&rounded);
        return true;
    }

    /*
     * For input like 10000000000, we must treat stripped digits as real. So
     * the loop assumes there are weight+1 digits before the decimal point.
     */
    weight = rounded.weight;
    assert(weight >= 0 && ndigits <= weight + 1);
    
    /*
     * Construct the result. To avoid issues with converting a value
     * corresponding to INT64_MIN (which can't be represented as a positive 64
     * bit two's complement integer), accumulate value as a negative number.
     */
    digits = rounded.digits;
    neg = (rounded.sign == NUMERIC_NEG);
    val = -digits[0];
    for (i = 1; i <= weight; i++)
    {
        if (unlikely(pg_mul_s64_overflow(val, NBASE, &val)))
        {
            free_var(&rounded);
            return false;
        }

        if (i < ndigits)
        {
            if (unlikely(pg_sub_s64_overflow(val, digits[i], &val)))
            {
                free_var(&rounded);
                return false;
            }
        }
    }

    free_var(&rounded);

    if (!neg)
    {
        if (unlikely(val == PG_INT64_MIN))
            return false;
        val = -val;
    }
    *result = val;

    return true;
}

/*
 * Convert int8 value to numeric.
 */
void
int64_to_numericvar(int64_t val, numeric *var)
{
    uint64_t		uval,
                newuval;
    NumericDigit *ptr;
    int			ndigits;

    /* int64 can require at most 19 decimal digits; add one for safety */
    alloc_var(var, 20 / DEC_DIGITS);
    if (val < 0)
    {
        var->sign = NUMERIC_NEG;
        uval = -val;
    }
    else
    {
        var->sign = NUMERIC_POS;
        uval = val;
    }
    var->dscale = 0;
    if (val == 0)
    {
        var->ndigits = 0;
        var->weight = 0;
        return;
    }
    ptr = var->digits + var->ndigits;
    ndigits = 0;
    do
    {
        ptr--;
        ndigits++;
        newuval = uval / NBASE;
        *ptr = uval - newuval * NBASE;
        uval = newuval;
    } while (uval);
    var->digits = ptr;
    var->ndigits = ndigits;
    var->weight = ndigits - 1;
}

/* As above, but work from a NumericVar */
double
numericvar_to_double_no_overflow(const numeric *var)
{
    char	   *tmp;
    double		val;
    char	   *endptr;

    tmp = get_str_from_var(var);

    /* unlike float8in, we ignore ERANGE from strtod */
    val = strtod(tmp, &endptr);
    if (*endptr != '\0')
    {
        /* shouldn't happen ... */
        throw pq_async::exception(
            "invalid input syntax for type double precision"
        );
    }

    free(tmp);

    return val;
}


/*
 * cmp_var() -
 *
 *	Compare two values on variable level.  We assume zeroes have been
 *	truncated to no digits.
 */
int
cmp_var(const numeric *var1, const numeric *var2)
{
    return cmp_var_common(var1->digits, var1->ndigits,
                          var1->weight, var1->sign,
                          var2->digits, var2->ndigits,
                          var2->weight, var2->sign);
}

int cmp_var_common(
    const NumericDigit* var1digits, int var1ndigits,
    int var1weight, int var1sign,
    const NumericDigit* var2digits, int var2ndigits,
    int var2weight, int var2sign)
{
    if(var1ndigits == 0){
        if(var2ndigits == 0)
            return 0;
        if (var2sign == NUMERIC_NEG)
            return 1;
        return -1;
    }
    if(var2ndigits == 0){
        if (var1sign == NUMERIC_POS)
            return 1;
        return -1;
    }

    if(var1sign == NUMERIC_POS){
        if(var2sign == NUMERIC_NEG)
            return 1;
        
        return cmp_abs_common(
            var1digits, var1ndigits, var1weight,
            var2digits, var2ndigits, var2weight
        );
    }

    if (var2sign == NUMERIC_POS)
        return -1;

    return cmp_abs_common(
        var2digits, var2ndigits, var2weight,
        var1digits, var1ndigits, var1weight
    );
}








/*
 * add_var() -
 *
 *	Full version of add functionality on variable level (handling signs).
 *	result might point to one of the operands too without danger.
 */
void
add_var(const numeric *var1, const numeric *var2, numeric *result)
{
    /*
     * Decide on the signs of the two variables what to do
     */
    if (var1->sign == NUMERIC_POS)
    {
        if (var2->sign == NUMERIC_POS)
        {
            /*
             * Both are positive result = +(ABS(var1) + ABS(var2))
             */
            add_abs(var1, var2, result);
            result->sign = NUMERIC_POS;
        }
        else
        {
            /*
             * var1 is positive, var2 is negative Must compare absolute values
             */
            switch (cmp_abs(var1, var2))
            {
                case 0:
                    /* ----------
                     * ABS(var1) == ABS(var2)
                     * result = ZERO
                     * ----------
                     */
                    zero_var(result);
                    result->dscale = std::max(var1->dscale, var2->dscale);
                    break;

                case 1:
                    /* ----------
                     * ABS(var1) > ABS(var2)
                     * result = +(ABS(var1) - ABS(var2))
                     * ----------
                     */
                    sub_abs(var1, var2, result);
                    result->sign = NUMERIC_POS;
                    break;

                case -1:
                    /* ----------
                     * ABS(var1) < ABS(var2)
                     * result = -(ABS(var2) - ABS(var1))
                     * ----------
                     */
                    sub_abs(var2, var1, result);
                    result->sign = NUMERIC_NEG;
                    break;
            }
        }
    }
    else
    {
        if (var2->sign == NUMERIC_POS)
        {
            /* ----------
             * var1 is negative, var2 is positive
             * Must compare absolute values
             * ----------
             */
            switch (cmp_abs(var1, var2))
            {
                case 0:
                    /* ----------
                     * ABS(var1) == ABS(var2)
                     * result = ZERO
                     * ----------
                     */
                    zero_var(result);
                    result->dscale = std::max(var1->dscale, var2->dscale);
                    break;

                case 1:
                    /* ----------
                     * ABS(var1) > ABS(var2)
                     * result = -(ABS(var1) - ABS(var2))
                     * ----------
                     */
                    sub_abs(var1, var2, result);
                    result->sign = NUMERIC_NEG;
                    break;

                case -1:
                    /* ----------
                     * ABS(var1) < ABS(var2)
                     * result = +(ABS(var2) - ABS(var1))
                     * ----------
                     */
                    sub_abs(var2, var1, result);
                    result->sign = NUMERIC_POS;
                    break;
            }
        }
        else
        {
            /* ----------
             * Both are negative
             * result = -(ABS(var1) + ABS(var2))
             * ----------
             */
            add_abs(var1, var2, result);
            result->sign = NUMERIC_NEG;
        }
    }
}


/*
 * sub_var() -
 *
 *	Full version of sub functionality on variable level (handling signs).
 *	result might point to one of the operands too without danger.
 */
void
sub_var(const numeric *var1, const numeric *var2, numeric *result)
{
    /*
     * Decide on the signs of the two variables what to do
     */
    if (var1->sign == NUMERIC_POS)
    {
        if (var2->sign == NUMERIC_NEG)
        {
            /* ----------
             * var1 is positive, var2 is negative
             * result = +(ABS(var1) + ABS(var2))
             * ----------
             */
            add_abs(var1, var2, result);
            result->sign = NUMERIC_POS;
        }
        else
        {
            /* ----------
             * Both are positive
             * Must compare absolute values
             * ----------
             */
            switch (cmp_abs(var1, var2))
            {
                case 0:
                    /* ----------
                     * ABS(var1) == ABS(var2)
                     * result = ZERO
                     * ----------
                     */
                    zero_var(result);
                    result->dscale = std::max(var1->dscale, var2->dscale);
                    break;

                case 1:
                    /* ----------
                     * ABS(var1) > ABS(var2)
                     * result = +(ABS(var1) - ABS(var2))
                     * ----------
                     */
                    sub_abs(var1, var2, result);
                    result->sign = NUMERIC_POS;
                    break;

                case -1:
                    /* ----------
                     * ABS(var1) < ABS(var2)
                     * result = -(ABS(var2) - ABS(var1))
                     * ----------
                     */
                    sub_abs(var2, var1, result);
                    result->sign = NUMERIC_NEG;
                    break;
            }
        }
    }
    else
    {
        if (var2->sign == NUMERIC_NEG)
        {
            /* ----------
             * Both are negative
             * Must compare absolute values
             * ----------
             */
            switch (cmp_abs(var1, var2))
            {
                case 0:
                    /* ----------
                     * ABS(var1) == ABS(var2)
                     * result = ZERO
                     * ----------
                     */
                    zero_var(result);
                    result->dscale = std::max(var1->dscale, var2->dscale);
                    break;

                case 1:
                    /* ----------
                     * ABS(var1) > ABS(var2)
                     * result = -(ABS(var1) - ABS(var2))
                     * ----------
                     */
                    sub_abs(var1, var2, result);
                    result->sign = NUMERIC_NEG;
                    break;

                case -1:
                    /* ----------
                     * ABS(var1) < ABS(var2)
                     * result = +(ABS(var2) - ABS(var1))
                     * ----------
                     */
                    sub_abs(var2, var1, result);
                    result->sign = NUMERIC_POS;
                    break;
            }
        }
        else
        {
            /* ----------
             * var1 is negative, var2 is positive
             * result = -(ABS(var1) + ABS(var2))
             * ----------
             */
            add_abs(var1, var2, result);
            result->sign = NUMERIC_NEG;
        }
    }
}


/*
 * mul_var() -
 *
 *	Multiplication on variable level. Product of var1 * var2 is stored
 *	in result.  Result is rounded to no more than rscale fractional digits.
 */
void
mul_var(const numeric *var1, const numeric *var2, numeric *result,
        int rscale)
{
    int			res_ndigits;
    int			res_sign;
    int			res_weight;
    int			maxdigits;
    int		   *dig;
    int			carry;
    int			maxdig;
    int			newdig;
    int			var1ndigits;
    int			var2ndigits;
    NumericDigit *var1digits;
    NumericDigit *var2digits;
    NumericDigit *res_digits;
    int			i,
                i1,
                i2;

    /*
     * Arrange for var1 to be the shorter of the two numbers.  This improves
     * performance because the inner multiplication loop is much simpler than
     * the outer loop, so it's better to have a smaller number of iterations
     * of the outer loop.  This also reduces the number of times that the
     * accumulator array needs to be normalized.
     */
    if (var1->ndigits > var2->ndigits)
    {
        const numeric *tmp = var1;

        var1 = var2;
        var2 = tmp;
    }

    /* copy these values into local vars for speed in inner loop */
    var1ndigits = var1->ndigits;
    var2ndigits = var2->ndigits;
    var1digits = var1->digits;
    var2digits = var2->digits;

    if (var1ndigits == 0 || var2ndigits == 0)
    {
        /* one or both inputs is zero; so is result */
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /* Determine result sign and (maximum possible) weight */
    if (var1->sign == var2->sign)
        res_sign = NUMERIC_POS;
    else
        res_sign = NUMERIC_NEG;
    res_weight = var1->weight + var2->weight + 2;

    /*
     * Determine the number of result digits to compute.  If the exact result
     * would have more than rscale fractional digits, truncate the computation
     * with MUL_GUARD_DIGITS guard digits, i.e., ignore input digits that
     * would only contribute to the right of that.  (This will give the exact
     * rounded-to-rscale answer unless carries out of the ignored positions
     * would have propagated through more than MUL_GUARD_DIGITS digits.)
     *
     * Note: an exact computation could not produce more than var1ndigits +
     * var2ndigits digits, but we allocate one extra output digit in case
     * rscale-driven rounding produces a carry out of the highest exact digit.
     */
    res_ndigits = var1ndigits + var2ndigits + 1;
    maxdigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS +
        MUL_GUARD_DIGITS;
    res_ndigits = std::min(res_ndigits, maxdigits);

    if (res_ndigits < 3)
    {
        /* All input digits will be ignored; so result is zero */
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * We do the arithmetic in an array "dig[]" of signed int's.  Since
     * INT_MAX is noticeably larger than NBASE*NBASE, this gives us headroom
     * to avoid normalizing carries immediately.
     *
     * maxdig tracks the maximum possible value of any dig[] entry; when this
     * threatens to exceed INT_MAX, we take the time to propagate carries.
     * Furthermore, we need to ensure that overflow doesn't occur during the
     * carry propagation passes either.  The carry values could be as much as
     * INT_MAX/NBASE, so really we must normalize when digits threaten to
     * exceed INT_MAX - INT_MAX/NBASE.
     *
     * To avoid overflow in maxdig itself, it actually represents the max
     * possible value divided by NBASE-1, ie, at the top of the loop it is
     * known that no dig[] entry exceeds maxdig * (NBASE-1).
     */
    dig = (int *) malloc(res_ndigits * sizeof(int));
    memset(dig, 0, res_ndigits * sizeof(int));
    maxdig = 0;

    /*
     * The least significant digits of var1 should be ignored if they don't
     * contribute directly to the first res_ndigits digits of the result that
     * we are computing.
     *
     * Digit i1 of var1 and digit i2 of var2 are multiplied and added to digit
     * i1+i2+2 of the accumulator array, so we need only consider digits of
     * var1 for which i1 <= res_ndigits - 3.
     */
    for (i1 = std::min(var1ndigits - 1, res_ndigits - 3); i1 >= 0; i1--)
    {
        int			var1digit = var1digits[i1];

        if (var1digit == 0)
            continue;

        /* Time to normalize? */
        maxdig += var1digit;
        if (maxdig > (INT_MAX - INT_MAX / NBASE) / (NBASE - 1))
        {
            /* Yes, do it */
            carry = 0;
            for (i = res_ndigits - 1; i >= 0; i--)
            {
                newdig = dig[i] + carry;
                if (newdig >= NBASE)
                {
                    carry = newdig / NBASE;
                    newdig -= carry * NBASE;
                }
                else
                    carry = 0;
                dig[i] = newdig;
            }
            assert(carry == 0);
            /* Reset maxdig to indicate new worst-case */
            maxdig = 1 + var1digit;
        }

        /*
         * Add the appropriate multiple of var2 into the accumulator.
         *
         * As above, digits of var2 can be ignored if they don't contribute,
         * so we only include digits for which i1+i2+2 <= res_ndigits - 1.
         */
        for (i2 = std::min(var2ndigits - 1, res_ndigits - i1 - 3), i = i1 + i2 + 2;
             i2 >= 0; i2--)
            dig[i--] += var1digit * var2digits[i2];
    }

    /*
     * Now we do a final carry propagation pass to normalize the result, which
     * we combine with storing the result digits into the output. Note that
     * this is still done at full precision w/guard digits.
     */
    alloc_var(result, res_ndigits);
    res_digits = result->digits;
    carry = 0;
    for (i = res_ndigits - 1; i >= 0; i--)
    {
        newdig = dig[i] + carry;
        if (newdig >= NBASE)
        {
            carry = newdig / NBASE;
            newdig -= carry * NBASE;
        }
        else
            carry = 0;
        res_digits[i] = newdig;
    }
    assert(carry == 0);
    
    free(dig);
    
    /*
     * Finally, round the result to the requested precision.
     */
    result->weight = res_weight;
    result->sign = res_sign;

    /* Round to target rscale (and set result->dscale) */
    round_var(result, rscale);

    /* Strip leading and trailing zeroes */
    strip_var(result);
}


/*
 * div_var() -
 *
 *	Division on variable level. Quotient of var1 / var2 is stored in result.
 *	The quotient is figured to exactly rscale fractional digits.
 *	If round is true, it is rounded at the rscale'th digit; if false, it
 *	is truncated (towards zero) at that digit.
 */
void
div_var(const numeric *var1, const numeric *var2, numeric *result,
        int rscale, bool round)
{
    int			div_ndigits;
    int			res_ndigits;
    int			res_sign;
    int			res_weight;
    int			carry;
    int			borrow;
    int			divisor1;
    int			divisor2;
    NumericDigit *dividend;
    NumericDigit *divisor;
    NumericDigit *res_digits;
    int			i;
    int			j;

    /* copy these values into local vars for speed in inner loop */
    int			var1ndigits = var1->ndigits;
    int			var2ndigits = var2->ndigits;

    /*
     * First of all division by zero check; we must not be handed an
     * unnormalized divisor.
     */
    if (var2ndigits == 0 || var2->digits[0] == 0)
        throw pq_async::exception("division by zero");

    /*
     * Now result zero check
     */
    if (var1ndigits == 0)
    {
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * Determine the result sign, weight and number of digits to calculate.
     * The weight figured here is correct if the emitted quotient has no
     * leading zero digits; otherwise strip_var() will fix things up.
     */
    if (var1->sign == var2->sign)
        res_sign = NUMERIC_POS;
    else
        res_sign = NUMERIC_NEG;
    res_weight = var1->weight - var2->weight;
    /* The number of accurate result digits we need to produce: */
    res_ndigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS;
    /* ... but always at least 1 */
    res_ndigits = std::max(res_ndigits, 1);
    /* If rounding needed, figure one more digit to ensure correct result */
    if (round)
        res_ndigits++;

    /*
     * The working dividend normally requires res_ndigits + var2ndigits
     * digits, but make it at least var1ndigits so we can load all of var1
     * into it.  (There will be an additional digit dividend[0] in the
     * dividend space, but for consistency with Knuth's notation we don't
     * count that in div_ndigits.)
     */
    div_ndigits = res_ndigits + var2ndigits;
    div_ndigits = std::max(div_ndigits, var1ndigits);

    /*
     * We need a workspace with room for the working dividend (div_ndigits+1
     * digits) plus room for the possibly-normalized divisor (var2ndigits
     * digits).  It is convenient also to have a zero at divisor[0] with the
     * actual divisor data in divisor[1 .. var2ndigits].  Transferring the
     * digits into the workspace also allows us to realloc the result (which
     * might be the same as either input var) before we begin the main loop.
     * Note that we use palloc0 to ensure that divisor[0], dividend[0], and
     * any additional dividend positions beyond var1ndigits, start out 0.
     */
    dividend = (NumericDigit *)
        malloc((div_ndigits + var2ndigits + 2) * sizeof(NumericDigit));
    memset(dividend, 0, (div_ndigits + var2ndigits + 2) * sizeof(NumericDigit));
    
    divisor = dividend + (div_ndigits + 1);
    memcpy(dividend + 1, var1->digits, var1ndigits * sizeof(NumericDigit));
    memcpy(divisor + 1, var2->digits, var2ndigits * sizeof(NumericDigit));

    /*
     * Now we can realloc the result to hold the generated quotient digits.
     */
    alloc_var(result, res_ndigits);
    res_digits = result->digits;

    if (var2ndigits == 1)
    {
        /*
         * If there's only a single divisor digit, we can use a fast path (cf.
         * Knuth section 4.3.1 exercise 16).
         */
        divisor1 = divisor[1];
        carry = 0;
        for (i = 0; i < res_ndigits; i++)
        {
            carry = carry * NBASE + dividend[i + 1];
            res_digits[i] = carry / divisor1;
            carry = carry % divisor1;
        }
    }
    else
    {
        /*
         * The full multiple-place algorithm is taken from Knuth volume 2,
         * Algorithm 4.3.1D.
         *
         * We need the first divisor digit to be >= NBASE/2.  If it isn't,
         * make it so by scaling up both the divisor and dividend by the
         * factor "d".  (The reason for allocating dividend[0] above is to
         * leave room for possible carry here.)
         */
        if (divisor[1] < HALF_NBASE)
        {
            int			d = NBASE / (divisor[1] + 1);

            carry = 0;
            for (i = var2ndigits; i > 0; i--)
            {
                carry += divisor[i] * d;
                divisor[i] = carry % NBASE;
                carry = carry / NBASE;
            }
            assert(carry == 0);
            carry = 0;
            /* at this point only var1ndigits of dividend can be nonzero */
            for (i = var1ndigits; i >= 0; i--)
            {
                carry += dividend[i] * d;
                dividend[i] = carry % NBASE;
                carry = carry / NBASE;
            }
            assert(carry == 0);
            assert(divisor[1] >= HALF_NBASE);
        }
        /* First 2 divisor digits are used repeatedly in main loop */
        divisor1 = divisor[1];
        divisor2 = divisor[2];

        /*
         * Begin the main loop.  Each iteration of this loop produces the j'th
         * quotient digit by dividing dividend[j .. j + var2ndigits] by the
         * divisor; this is essentially the same as the common manual
         * procedure for long division.
         */
        for (j = 0; j < res_ndigits; j++)
        {
            /* Estimate quotient digit from the first two dividend digits */
            int			next2digits = dividend[j] * NBASE + dividend[j + 1];
            int			qhat;

            /*
             * If next2digits are 0, then quotient digit must be 0 and there's
             * no need to adjust the working dividend.  It's worth testing
             * here to fall out ASAP when processing trailing zeroes in a
             * dividend.
             */
            if (next2digits == 0)
            {
                res_digits[j] = 0;
                continue;
            }

            if (dividend[j] == divisor1)
                qhat = NBASE - 1;
            else
                qhat = next2digits / divisor1;

            /*
             * Adjust quotient digit if it's too large.  Knuth proves that
             * after this step, the quotient digit will be either correct or
             * just one too large.  (Note: it's OK to use dividend[j+2] here
             * because we know the divisor length is at least 2.)
             */
            while (divisor2 * qhat >
                   (next2digits - qhat * divisor1) * NBASE + dividend[j + 2])
                qhat--;

            /* As above, need do nothing more when quotient digit is 0 */
            if (qhat > 0)
            {
                /*
                 * Multiply the divisor by qhat, and subtract that from the
                 * working dividend.  "carry" tracks the multiplication,
                 * "borrow" the subtraction (could we fold these together?)
                 */
                carry = 0;
                borrow = 0;
                for (i = var2ndigits; i >= 0; i--)
                {
                    carry += divisor[i] * qhat;
                    borrow -= carry % NBASE;
                    carry = carry / NBASE;
                    borrow += dividend[j + i];
                    if (borrow < 0)
                    {
                        dividend[j + i] = borrow + NBASE;
                        borrow = -1;
                    }
                    else
                    {
                        dividend[j + i] = borrow;
                        borrow = 0;
                    }
                }
                assert(carry == 0);

                /*
                 * If we got a borrow out of the top dividend digit, then
                 * indeed qhat was one too large.  Fix it, and add back the
                 * divisor to correct the working dividend.  (Knuth proves
                 * that this will occur only about 3/NBASE of the time; hence,
                 * it's a good idea to test this code with small NBASE to be
                 * sure this section gets exercised.)
                 */
                if (borrow)
                {
                    qhat--;
                    carry = 0;
                    for (i = var2ndigits; i >= 0; i--)
                    {
                        carry += dividend[j + i] + divisor[i];
                        if (carry >= NBASE)
                        {
                            dividend[j + i] = carry - NBASE;
                            carry = 1;
                        }
                        else
                        {
                            dividend[j + i] = carry;
                            carry = 0;
                        }
                    }
                    /* A carry should occur here to cancel the borrow above */
                    assert(carry == 1);
                }
            }

            /* And we're done with this quotient digit */
            res_digits[j] = qhat;
        }
    }
    
    free(dividend);
    
    /*
     * Finally, round or truncate the result to the requested precision.
     */
    result->weight = res_weight;
    result->sign = res_sign;

    /* Round or truncate to target rscale (and set result->dscale) */
    if (round)
        round_var(result, rscale);
    else
        trunc_var(result, rscale);

    /* Strip leading and trailing zeroes */
    strip_var(result);
}


/*
 * div_var_fast() -
 *
 *	This has the same API as div_var, but is implemented using the division
 *	algorithm from the "FM" library, rather than Knuth's schoolbook-division
 *	approach.  This is significantly faster but can produce inaccurate
 *	results, because it sometimes has to propagate rounding to the left,
 *	and so we can never be entirely sure that we know the requested digits
 *	exactly.  We compute DIV_GUARD_DIGITS extra digits, but there is
 *	no certainty that that's enough.  We use this only in the transcendental
 *	function calculation routines, where everything is approximate anyway.
 *
 *	Although we provide a "round" argument for consistency with div_var,
 *	it is unwise to use this function with round=false.  In truncation mode
 *	it is possible to get a result with no significant digits, for example
 *	with rscale=0 we might compute 0.99999... and truncate that to 0 when
 *	the correct answer is 1.
 */
void
div_var_fast(const numeric *var1, const numeric *var2,
             numeric *result, int rscale, bool round)
{
    int			div_ndigits;
    int			res_sign;
    int			res_weight;
    int		   *div;
    int			qdigit;
    int			carry;
    int			maxdiv;
    int			newdig;
    NumericDigit *res_digits;
    double		fdividend,
                fdivisor,
                fdivisorinverse,
                fquotient;
    int			qi;
    int			i;

    /* copy these values into local vars for speed in inner loop */
    int			var1ndigits = var1->ndigits;
    int			var2ndigits = var2->ndigits;
    NumericDigit *var1digits = var1->digits;
    NumericDigit *var2digits = var2->digits;

    /*
     * First of all division by zero check; we must not be handed an
     * unnormalized divisor.
     */
    if (var2ndigits == 0 || var2digits[0] == 0)
        throw pq_async::exception("division by zero");

    /*
     * Now result zero check
     */
    if (var1ndigits == 0)
    {
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * Determine the result sign, weight and number of digits to calculate
     */
    if (var1->sign == var2->sign)
        res_sign = NUMERIC_POS;
    else
        res_sign = NUMERIC_NEG;
    res_weight = var1->weight - var2->weight + 1;
    /* The number of accurate result digits we need to produce: */
    div_ndigits = res_weight + 1 + (rscale + DEC_DIGITS - 1) / DEC_DIGITS;
    /* Add guard digits for roundoff error */
    div_ndigits += DIV_GUARD_DIGITS;
    if (div_ndigits < DIV_GUARD_DIGITS)
        div_ndigits = DIV_GUARD_DIGITS;
    /* Must be at least var1ndigits, too, to simplify data-loading loop */
    if (div_ndigits < var1ndigits)
        div_ndigits = var1ndigits;

    /*
     * We do the arithmetic in an array "div[]" of signed int's.  Since
     * INT_MAX is noticeably larger than NBASE*NBASE, this gives us headroom
     * to avoid normalizing carries immediately.
     *
     * We start with div[] containing one zero digit followed by the
     * dividend's digits (plus appended zeroes to reach the desired precision
     * including guard digits).  Each step of the main loop computes an
     * (approximate) quotient digit and stores it into div[], removing one
     * position of dividend space.  A final pass of carry propagation takes
     * care of any mistaken quotient digits.
     */
    div = (int *) malloc((div_ndigits + 1) * sizeof(int));
    memset(div, 0, (div_ndigits + 1) * sizeof(int));
    
    for (i = 0; i < var1ndigits; i++)
        div[i + 1] = var1digits[i];

    /*
     * We estimate each quotient digit using floating-point arithmetic, taking
     * the first four digits of the (current) dividend and divisor.  This must
     * be float to avoid overflow.  The quotient digits will generally be off
     * by no more than one from the exact answer.
     */
    fdivisor = (double) var2digits[0];
    for (i = 1; i < 4; i++)
    {
        fdivisor *= NBASE;
        if (i < var2ndigits)
            fdivisor += (double) var2digits[i];
    }
    fdivisorinverse = 1.0 / fdivisor;

    /*
     * maxdiv tracks the maximum possible absolute value of any div[] entry;
     * when this threatens to exceed INT_MAX, we take the time to propagate
     * carries.  Furthermore, we need to ensure that overflow doesn't occur
     * during the carry propagation passes either.  The carry values may have
     * an absolute value as high as INT_MAX/NBASE + 1, so really we must
     * normalize when digits threaten to exceed INT_MAX - INT_MAX/NBASE - 1.
     *
     * To avoid overflow in maxdiv itself, it represents the max absolute
     * value divided by NBASE-1, ie, at the top of the loop it is known that
     * no div[] entry has an absolute value exceeding maxdiv * (NBASE-1).
     *
     * Actually, though, that holds good only for div[] entries after div[qi];
     * the adjustment done at the bottom of the loop may cause div[qi + 1] to
     * exceed the maxdiv limit, so that div[qi] in the next iteration is
     * beyond the limit.  This does not cause problems, as explained below.
     */
    maxdiv = 1;

    /*
     * Outer loop computes next quotient digit, which will go into div[qi]
     */
    for (qi = 0; qi < div_ndigits; qi++)
    {
        /* Approximate the current dividend value */
        fdividend = (double) div[qi];
        for (i = 1; i < 4; i++)
        {
            fdividend *= NBASE;
            if (qi + i <= div_ndigits)
                fdividend += (double) div[qi + i];
        }
        /* Compute the (approximate) quotient digit */
        fquotient = fdividend * fdivisorinverse;
        qdigit = (fquotient >= 0.0) ? ((int) fquotient) :
            (((int) fquotient) - 1);	/* truncate towards -infinity */

        if (qdigit != 0)
        {
            /* Do we need to normalize now? */
            maxdiv += std::abs(qdigit);
            if (maxdiv > (INT_MAX - INT_MAX / NBASE - 1) / (NBASE - 1))
            {
                /* Yes, do it */
                carry = 0;
                for (i = div_ndigits; i > qi; i--)
                {
                    newdig = div[i] + carry;
                    if (newdig < 0)
                    {
                        carry = -((-newdig - 1) / NBASE) - 1;
                        newdig -= carry * NBASE;
                    }
                    else if (newdig >= NBASE)
                    {
                        carry = newdig / NBASE;
                        newdig -= carry * NBASE;
                    }
                    else
                        carry = 0;
                    div[i] = newdig;
                }
                newdig = div[qi] + carry;
                div[qi] = newdig;

                /*
                 * All the div[] digits except possibly div[qi] are now in the
                 * range 0..NBASE-1.  We do not need to consider div[qi] in
                 * the maxdiv value anymore, so we can reset maxdiv to 1.
                 */
                maxdiv = 1;

                /*
                 * Recompute the quotient digit since new info may have
                 * propagated into the top four dividend digits
                 */
                fdividend = (double) div[qi];
                for (i = 1; i < 4; i++)
                {
                    fdividend *= NBASE;
                    if (qi + i <= div_ndigits)
                        fdividend += (double) div[qi + i];
                }
                /* Compute the (approximate) quotient digit */
                fquotient = fdividend * fdivisorinverse;
                qdigit = (fquotient >= 0.0) ? ((int) fquotient) :
                    (((int) fquotient) - 1);	/* truncate towards -infinity */
                maxdiv += std::abs(qdigit);
            }

            /*
             * Subtract off the appropriate multiple of the divisor.
             *
             * The digits beyond div[qi] cannot overflow, because we know they
             * will fall within the maxdiv limit.  As for div[qi] itself, note
             * that qdigit is approximately trunc(div[qi] / vardigits[0]),
             * which would make the new value simply div[qi] mod vardigits[0].
             * The lower-order terms in qdigit can change this result by not
             * more than about twice INT_MAX/NBASE, so overflow is impossible.
             */
            if (qdigit != 0)
            {
                int			istop = std::min(var2ndigits, div_ndigits - qi + 1);

                for (i = 0; i < istop; i++)
                    div[qi + i] -= qdigit * var2digits[i];
            }
        }

        /*
         * The dividend digit we are about to replace might still be nonzero.
         * Fold it into the next digit position.
         *
         * There is no risk of overflow here, although proving that requires
         * some care.  Much as with the argument for div[qi] not overflowing,
         * if we consider the first two terms in the numerator and denominator
         * of qdigit, we can see that the final value of div[qi + 1] will be
         * approximately a remainder mod (vardigits[0]*NBASE + vardigits[1]).
         * Accounting for the lower-order terms is a bit complicated but ends
         * up adding not much more than INT_MAX/NBASE to the possible range.
         * Thus, div[qi + 1] cannot overflow here, and in its role as div[qi]
         * in the next loop iteration, it can't be large enough to cause
         * overflow in the carry propagation step (if any), either.
         *
         * But having said that: div[qi] can be more than INT_MAX/NBASE, as
         * noted above, which means that the product div[qi] * NBASE *can*
         * overflow.  When that happens, adding it to div[qi + 1] will always
         * cause a canceling overflow so that the end result is correct.  We
         * could avoid the intermediate overflow by doing the multiplication
         * and addition in int64 arithmetic, but so far there appears no need.
         */
        div[qi + 1] += div[qi] * NBASE;

        div[qi] = qdigit;
    }

    /*
     * Approximate and store the last quotient digit (div[div_ndigits])
     */
    fdividend = (double) div[qi];
    for (i = 1; i < 4; i++)
        fdividend *= NBASE;
    fquotient = fdividend * fdivisorinverse;
    qdigit = (fquotient >= 0.0) ? ((int) fquotient) :
        (((int) fquotient) - 1);	/* truncate towards -infinity */
    div[qi] = qdigit;

    /*
     * Because the quotient digits might be off by one, some of them might be
     * -1 or NBASE at this point.  The represented value is correct in a
     * mathematical sense, but it doesn't look right.  We do a final carry
     * propagation pass to normalize the digits, which we combine with storing
     * the result digits into the output.  Note that this is still done at
     * full precision w/guard digits.
     */
    alloc_var(result, div_ndigits + 1);
    res_digits = result->digits;
    carry = 0;
    for (i = div_ndigits; i >= 0; i--)
    {
        newdig = div[i] + carry;
        if (newdig < 0)
        {
            carry = -((-newdig - 1) / NBASE) - 1;
            newdig -= carry * NBASE;
        }
        else if (newdig >= NBASE)
        {
            carry = newdig / NBASE;
            newdig -= carry * NBASE;
        }
        else
            carry = 0;
        res_digits[i] = newdig;
    }
    assert(carry == 0);

    free(div);

    /*
     * Finally, round the result to the requested precision.
     */
    result->weight = res_weight;
    result->sign = res_sign;

    /* Round to target rscale (and set result->dscale) */
    if (round)
        round_var(result, rscale);
    else
        trunc_var(result, rscale);

    /* Strip leading and trailing zeroes */
    strip_var(result);
}

/*
 * Default scale selection for division
 *
 * Returns the appropriate result scale for the division result.
 */
int
select_div_scale(const numeric *var1, const numeric *var2)
{
    int			weight1,
                weight2,
                qweight,
                i;
    NumericDigit firstdigit1,
                firstdigit2;
    int			rscale;

    /*
     * The result scale of a division isn't specified in any SQL standard. For
     * PostgreSQL we select a result scale that will give at least
     * NUMERIC_MIN_SIG_DIGITS significant digits, so that numeric gives a
     * result no less accurate than float8; but use a scale not less than
     * either input's display scale.
     */

    /* Get the actual (normalized) weight and first digit of each input */

    weight1 = 0;				/* values to use if var1 is zero */
    firstdigit1 = 0;
    for (i = 0; i < var1->ndigits; i++)
    {
        firstdigit1 = var1->digits[i];
        if (firstdigit1 != 0)
        {
            weight1 = var1->weight - i;
            break;
        }
    }

    weight2 = 0;				/* values to use if var2 is zero */
    firstdigit2 = 0;
    for (i = 0; i < var2->ndigits; i++)
    {
        firstdigit2 = var2->digits[i];
        if (firstdigit2 != 0)
        {
            weight2 = var2->weight - i;
            break;
        }
    }

    /*
     * Estimate weight of quotient.  If the two first digits are equal, we
     * can't be sure, but assume that var1 is less than var2.
     */
    qweight = weight1 - weight2;
    if (firstdigit1 <= firstdigit2)
        qweight--;

    /* Select result scale */
    rscale = NUMERIC_MIN_SIG_DIGITS - qweight * DEC_DIGITS;
    rscale = std::max(rscale, var1->dscale);
    rscale = std::max(rscale, var2->dscale);
    rscale = std::max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
    rscale = std::min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

    return rscale;
}


/*
 * mod_var() -
 *
 *	Calculate the modulo of two numerics at variable level
 */
void
mod_var(const numeric *var1, const numeric *var2, numeric *result)
{
    numeric	tmp;
    
    //init_var(&tmp);
    
    /* ---------
     * We do this using the equation
     *		mod(x,y) = x - trunc(x/y)*y
     * div_var can be persuaded to give us trunc(x/y) directly.
     * ----------
     */
    div_var(var1, var2, &tmp, 0, false);

    mul_var(var2, &tmp, &tmp, var2->dscale);

    sub_var(var1, &tmp, result);

    free_var(&tmp);
}


/*
 * ceil_var() -
 *
 *	Return the smallest integer greater than or equal to the argument
 *	on variable level
 */
void
ceil_var(const numeric *var, numeric *result)
{
    numeric	tmp(*var);
    
    //init_var(&tmp);
    //set_var_from_var(var, &tmp);

    trunc_var(&tmp, 0);

    if (var->sign == NUMERIC_POS && numeric::cmp(*var, tmp) != 0)
        add_var(&tmp, &const_one, &tmp);

    *result = tmp;
    //set_var_from_var(&tmp, result);
    
    free_var(&tmp);
}


/*
 * floor_var() -
 *
 *	Return the largest integer equal to or less than the argument
 *	on variable level
 */
void
floor_var(const numeric *var, numeric *result)
{
    numeric	tmp(*var);
    
    //init_var(&tmp);
    //set_var_from_var(var, &tmp);
    
    trunc_var(&tmp, 0);
    
    if (var->sign == NUMERIC_NEG && numeric::cmp(*var, tmp) != 0)
        sub_var(&tmp, &const_one, &tmp);
    
    *result = tmp;
    //set_var_from_var(&tmp, result);
    free_var(&tmp);
}










/*
 * sqrt_var() -
 *
 *	Compute the square root of x using Newton's algorithm
 */
void
sqrt_var(const numeric *arg, numeric *result, int rscale)
{
    numeric	tmp_arg;
    numeric	tmp_val;
    numeric	last_val;
    int			local_rscale;
    int			stat;

    local_rscale = rscale + 8;

    stat = cmp_var(arg, &const_zero);
    if (stat == 0)
    {
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * SQL2003 defines sqrt() in terms of power, so we need to emit the right
     * SQLSTATE error code if the operand is negative.
     */
    if (stat < 0)
        throw pq_async::exception(
            "cannot take square root of a negative number"
        );
    
    //init_var(&tmp_arg);
    //init_var(&tmp_val);
    //init_var(&last_val);
    
    /* Copy arg in case it is the same var as result */
    //set_var_from_var(arg, &tmp_arg);
    tmp_arg = *arg;
    
    /*
     * Initialize the result to the first guess
     */
    alloc_var(result, 1);
    result->digits[0] = tmp_arg.digits[0] / 2;
    if (result->digits[0] == 0)
        result->digits[0] = 1;
    result->weight = tmp_arg.weight / 2;
    result->sign = NUMERIC_POS;

    //set_var_from_var(result, &last_val);
    last_val = *result;
    
    for (;;)
    {
        div_var_fast(&tmp_arg, result, &tmp_val, local_rscale, true);

        add_var(result, &tmp_val, result);
        mul_var(result, &const_zero_point_five, result, local_rscale);
        
        if(cmp_var(&last_val, result) == 0)
            break;
        //set_var_from_var(result, &last_val);
        last_val = *result;
    }

    free_var(&last_val);
    free_var(&tmp_val);
    free_var(&tmp_arg);
    
    /* Round to requested precision */
    round_var(result, rscale);
}


/*
 * exp_var() -
 *
 *	Raise e to the power of x, computed to rscale fractional digits
 */
void
exp_var(const numeric *arg, numeric *result, int rscale)
{
    numeric	x;
    numeric	elem;
    numeric	ni;
    double		val;
    int			dweight;
    int			ndiv2;
    int			sig_digits;
    int			local_rscale;

    //init_var(&x);
    //init_var(&elem);
    //init_var(&ni);
    
    //set_var_from_var(arg, &x);
    x = *arg;
    
    /*
     * Estimate the dweight of the result using floating point arithmetic, so
     * that we can choose an appropriate local rscale for the calculation.
     */
    val = numericvar_to_double_no_overflow(&x);

    /* Guard against overflow */
    /* If you change this limit, see also power_var()'s limit */
    if (std::abs(val) >= NUMERIC_MAX_RESULT_SCALE * 3)
        throw pq_async::exception("value overflows numeric format");

    /* decimal weight = log10(e^x) = x * log10(e) */
    dweight = (int) (val * 0.434294481903252);

    /*
     * Reduce x to the range -0.01 <= x <= 0.01 (approximately) by dividing by
     * 2^n, to improve the convergence rate of the Taylor series.
     */
    if (std::abs(val) > 0.01)
    {
        numeric	tmp;

        //init_var(&tmp);
        //set_var_from_var(&const_two, &tmp);
        tmp = numeric::from(2);
        
        ndiv2 = 1;
        val /= 2;
        
        while (std::abs(val) > 0.01)
        {
            ndiv2++;
            val /= 2;
            add_var(&tmp, &tmp, &tmp);
        }

        local_rscale = x.dscale + ndiv2;
        div_var_fast(&x, &tmp, &x, local_rscale, true);

        free_var(&tmp);
    }
    else
        ndiv2 = 0;

    /*
     * Set the scale for the Taylor series expansion.  The final result has
     * (dweight + rscale + 1) significant digits.  In addition, we have to
     * raise the Taylor series result to the power 2^ndiv2, which introduces
     * an error of up to around log10(2^ndiv2) digits, so work with this many
     * extra digits of precision (plus a few more for good measure).
     */
    sig_digits = 1 + dweight + rscale + (int) (ndiv2 * 0.301029995663981);
    sig_digits = std::max(sig_digits, 0) + 8;
    
    local_rscale = sig_digits - 1;

    /*
     * Use the Taylor series
     *
     * exp(x) = 1 + x + x^2/2! + x^3/3! + ...
     *
     * Given the limited range of x, this should converge reasonably quickly.
     * We run the series until the terms fall below the local_rscale limit.
     */
    add_var(&const_one, &x, result);

    mul_var(&x, &x, &elem, local_rscale);
    //set_var_from_var(&const_two, &ni);
    ni = numeric::from(2);
    
    div_var_fast(&elem, &ni, &elem, local_rscale, true);

    while (elem.ndigits != 0)
    {
        add_var(result, &elem, result);

        mul_var(&elem, &x, &elem, local_rscale);
        add_var(&ni, &const_one, &ni);
        div_var_fast(&elem, &ni, &elem, local_rscale, true);
    }

    /*
     * Compensate for the argument range reduction.  Since the weight of the
     * result doubles with each multiplication, we can reduce the local rscale
     * as we proceed.
     */
    while (ndiv2-- > 0)
    {
        local_rscale = sig_digits - result->weight * 2 * DEC_DIGITS;
        local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);
        mul_var(result, result, result, local_rscale);
    }

    /* Round to requested rscale */
    round_var(result, rscale);

    free_var(&x);
    free_var(&elem);
    free_var(&ni);
}


/*
 * Estimate the dweight of the most significant decimal digit of the natural
 * logarithm of a number.
 *
 * Essentially, we're approximating log10(abs(ln(var))).  This is used to
 * determine the appropriate rscale when computing natural logarithms.
 */
int
estimate_ln_dweight(const numeric *var)
{
    int			ln_dweight;

    if (cmp_var(var, &const_zero_point_nine) >= 0 &&
        cmp_var(var, &const_one_point_one) <= 0)
    {
        /*
         * 0.9 <= var <= 1.1
         *
         * ln(var) has a negative weight (possibly very large).  To get a
         * reasonably accurate result, estimate it using ln(1+x) ~= x.
         */
        numeric	x;

        //init_var(&x);
        sub_var(var, &const_one, &x);
        
        if (x.ndigits > 0)
        {
            /* Use weight of most significant decimal digit of x */
            ln_dweight = x.weight * DEC_DIGITS + (int) log10(x.digits[0]);
        }
        else
        {
            /* x = 0.  Since ln(1) = 0 exactly, we don't need extra digits */
            ln_dweight = 0;
        }

        free_var(&x);
    }
    else
    {
        /*
         * Estimate the logarithm using the first couple of digits from the
         * input number.  This will give an accurate result whenever the input
         * is not too close to 1.
         */
        if (var->ndigits > 0)
        {
            int			digits;
            int			dweight;
            double		ln_var;

            digits = var->digits[0];
            dweight = var->weight * DEC_DIGITS;

            if (var->ndigits > 1)
            {
                digits = digits * NBASE + var->digits[1];
                dweight -= DEC_DIGITS;
            }

            /*----------
             * We have var ~= digits * 10^dweight
             * so ln(var) ~= ln(digits) + dweight * ln(10)
             *----------
             */
            ln_var = log((double) digits) + dweight * 2.302585092994046;
            ln_dweight = (int) log10(std::abs(ln_var));
        }
        else
        {
            /* Caller should fail on ln(0), but for the moment return zero */
            ln_dweight = 0;
        }
    }

    return ln_dweight;
}


/*
 * ln_var() -
 *
 *	Compute the natural log of x
 */
void
ln_var(const numeric *arg, numeric *result, int rscale)
{
    numeric	x;
    numeric	xx;
    numeric	ni;
    numeric	elem;
    numeric	fact;
    int			local_rscale;
    int			cmp;

    cmp = cmp_var(arg, &const_zero);
    if (cmp == 0)
        throw pq_async::exception("cannot take logarithm of zero");
    else if (cmp < 0)
        throw pq_async::exception(
            "cannot take logarithm of a negative number"
        );

    //init_var(&x);
    //init_var(&xx);
    //init_var(&ni);
    //init_var(&elem);
    //init_var(&fact);
    
    //set_var_from_var(arg, &x);
    x = *arg;
    //set_var_from_var(&const_two, &fact);
    fact = const_two;

    /*
     * Reduce input into range 0.9 < x < 1.1 with repeated sqrt() operations.
     *
     * The final logarithm will have up to around rscale+6 significant digits.
     * Each sqrt() will roughly halve the weight of x, so adjust the local
     * rscale as we work so that we keep this many significant digits at each
     * step (plus a few more for good measure).
     */
    while (cmp_var(&x, &const_zero_point_nine) <= 0)
    {
        local_rscale = rscale - x.weight * DEC_DIGITS / 2 + 8;
        local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);
        sqrt_var(&x, &x, local_rscale);
        mul_var(&fact, &const_two, &fact, 0);
    }
    while (cmp_var(&x, &const_one_point_one) >= 0)
    {
        local_rscale = rscale - x.weight * DEC_DIGITS / 2 + 8;
        local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);
        sqrt_var(&x, &x, local_rscale);
        mul_var(&fact, &const_two, &fact, 0);
    }

    /*
     * We use the Taylor series for 0.5 * ln((1+z)/(1-z)),
     *
     * z + z^3/3 + z^5/5 + ...
     *
     * where z = (x-1)/(x+1) is in the range (approximately) -0.053 .. 0.048
     * due to the above range-reduction of x.
     *
     * The convergence of this is not as fast as one would like, but is
     * tolerable given that z is small.
     */
    local_rscale = rscale + 8;

    sub_var(&x, &const_one, result);
    add_var(&x, &const_one, &elem);
    div_var_fast(result, &elem, result, local_rscale, true);
    //set_var_from_var(result, &xx);
    xx = *result;
    mul_var(result, result, &x, local_rscale);
    
    //set_var_from_var(&const_one, &ni);
    ni = const_one;
    
    for (;;)
    {
        add_var(&ni, &const_two, &ni);
        mul_var(&xx, &x, &xx, local_rscale);
        div_var_fast(&xx, &ni, &elem, local_rscale, true);

        if (elem.ndigits == 0)
            break;

        add_var(result, &elem, result);

        if (elem.weight < (result->weight - local_rscale * 2 / DEC_DIGITS))
            break;
    }

    /* Compensate for argument range reduction, round to requested rscale */
    mul_var(result, &fact, result, rscale);

    free_var(&x);
    free_var(&xx);
    free_var(&ni);
    free_var(&elem);
    free_var(&fact);
}


/*
 * log_var() -
 *
 *	Compute the logarithm of num in a given base.
 *
 *	Note: this routine chooses dscale of the result.
 */
void
log_var(const numeric *base, const numeric *num, numeric *result)
{
    numeric	ln_base;
    numeric	ln_num;
    int			ln_base_dweight;
    int			ln_num_dweight;
    int			result_dweight;
    int			rscale;
    int			ln_base_rscale;
    int			ln_num_rscale;

    //init_var(&ln_base);
    //init_var(&ln_num);

    /* Estimated dweights of ln(base), ln(num) and the final result */
    ln_base_dweight = estimate_ln_dweight(base);
    ln_num_dweight = estimate_ln_dweight(num);
    result_dweight = ln_num_dweight - ln_base_dweight;

    /*
     * Select the scale of the result so that it will have at least
     * NUMERIC_MIN_SIG_DIGITS significant digits and is not less than either
     * input's display scale.
     */
    rscale = NUMERIC_MIN_SIG_DIGITS - result_dweight;
    rscale = std::max(rscale, base->dscale);
    rscale = std::max(rscale, num->dscale);
    rscale = std::max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
    rscale = std::min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

    /*
     * Set the scales for ln(base) and ln(num) so that they each have more
     * significant digits than the final result.
     */
    ln_base_rscale = rscale + result_dweight - ln_base_dweight + 8;
    ln_base_rscale = std::max(ln_base_rscale, NUMERIC_MIN_DISPLAY_SCALE);

    ln_num_rscale = rscale + result_dweight - ln_num_dweight + 8;
    ln_num_rscale = std::max(ln_num_rscale, NUMERIC_MIN_DISPLAY_SCALE);

    /* Form natural logarithms */
    ln_var(base, &ln_base, ln_base_rscale);
    ln_var(num, &ln_num, ln_num_rscale);

    /* Divide and round to the required scale */
    div_var_fast(&ln_num, &ln_base, result, rscale, true);

    free_var(&ln_num);
    free_var(&ln_base);
}


/*
 * power_var() -
 *
 *	Raise base to the power of exp
 *
 *	Note: this routine chooses dscale of the result.
 */
void
power_var(const numeric *base, const numeric *exp, numeric *result)
{
    numeric	ln_base;
    numeric	ln_num;
    int			ln_dweight;
    int			rscale;
    int			local_rscale;
    double		val;

    /* If exp can be represented as an integer, use power_var_int */
    if (exp->ndigits == 0 || exp->ndigits <= exp->weight + 1)
    {
        /* exact integer, but does it fit in int? */
        int64_t		expval64;

        if (numericvar_to_int64(exp, &expval64))
        {
            int			expval = (int) expval64;

            /* Test for overflow by reverse-conversion. */
            if ((int64_t) expval == expval64)
            {
                /* Okay, select rscale */
                rscale = NUMERIC_MIN_SIG_DIGITS;
                rscale = std::max(rscale, base->dscale);
                rscale = std::max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
                rscale = std::min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

                power_var_int(base, expval, result, rscale);
                return;
            }
        }
    }

    /*
     * This avoids log(0) for cases of 0 raised to a non-integer.  0 ^ 0 is
     * handled by power_var_int().
     */
    if (cmp_var(base, &const_zero) == 0)
    {
        //set_var_from_var(&const_zero, result);
        *result = const_zero;
        result->dscale = NUMERIC_MIN_SIG_DIGITS;	/* no need to round */
        return;
    }

    //init_var(&ln_base);
    //init_var(&ln_num);

    /*----------
     * Decide on the scale for the ln() calculation.  For this we need an
     * estimate of the weight of the result, which we obtain by doing an
     * initial low-precision calculation of exp * ln(base).
     *
     * We want result = e ^ (exp * ln(base))
     * so result dweight = log10(result) = exp * ln(base) * log10(e)
     *
     * We also perform a crude overflow test here so that we can exit early if
     * the full-precision result is sure to overflow, and to guard against
     * integer overflow when determining the scale for the real calculation.
     * exp_var() supports inputs up to NUMERIC_MAX_RESULT_SCALE * 3, so the
     * result will overflow if exp * ln(base) >= NUMERIC_MAX_RESULT_SCALE * 3.
     * Since the values here are only approximations, we apply a small fuzz
     * factor to this overflow test and let exp_var() determine the exact
     * overflow threshold so that it is consistent for all inputs.
     *----------
     */
    ln_dweight = estimate_ln_dweight(base);

    local_rscale = 8 - ln_dweight;
    local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);
    local_rscale = std::min(local_rscale, NUMERIC_MAX_DISPLAY_SCALE);

    ln_var(base, &ln_base, local_rscale);

    mul_var(&ln_base, exp, &ln_num, local_rscale);

    val = numericvar_to_double_no_overflow(&ln_num);

    /* initial overflow test with fuzz factor */
    if (std::abs(val) > NUMERIC_MAX_RESULT_SCALE * 3.01)
        throw pq_async::exception("value overflows numeric format");

    val *= 0.434294481903252;	/* approximate decimal result weight */

    /* choose the result scale */
    rscale = NUMERIC_MIN_SIG_DIGITS - (int) val;
    rscale = std::max(rscale, base->dscale);
    rscale = std::max(rscale, exp->dscale);
    rscale = std::max(rscale, NUMERIC_MIN_DISPLAY_SCALE);
    rscale = std::min(rscale, NUMERIC_MAX_DISPLAY_SCALE);

    /* set the scale for the real exp * ln(base) calculation */
    local_rscale = rscale + (int) val - ln_dweight + 8;
    local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

    /* and do the real calculation */

    ln_var(base, &ln_base, local_rscale);

    mul_var(&ln_base, exp, &ln_num, local_rscale);

    exp_var(&ln_num, result, rscale);

    free_var(&ln_num);
    free_var(&ln_base);
}

/*
 * power_var_int() -
 *
 *	Raise base to the power of exp, where exp is an integer.
 */
void
power_var_int(const numeric *base, int exp, numeric *result, int rscale)
{
    double		f;
    int			p;
    int			i;
    int			sig_digits;
    unsigned int mask;
    bool		neg;
    numeric	base_prod;
    int			local_rscale;

    /* Handle some common special cases, as well as corner cases */
    switch (exp)
    {
        case 0:

            /*
             * While 0 ^ 0 can be either 1 or indeterminate (error), we treat
             * it as 1 because most programming languages do this. SQL:2003
             * also requires a return value of 1.
             * https://en.wikipedia.org/wiki/Exponentiation#Zero_to_the_zero_power
             */
            //set_var_from_var(&const_one, result);
            *result = const_one;
            result->dscale = rscale;	/* no need to round */
            return;
        case 1:
            //set_var_from_var(base, result);
            *result = *base;
            round_var(result, rscale);
            return;
        case -1:
            div_var(&const_one, base, result, rscale, true);
            return;
        case 2:
            mul_var(base, base, result, rscale);
            return;
        default:
            break;
    }

    /* Handle the special case where the base is zero */
    if (base->ndigits == 0)
    {
        if (exp < 0)
            throw pq_async::exception("division by zero");
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * The general case repeatedly multiplies base according to the bit
     * pattern of exp.
     *
     * First we need to estimate the weight of the result so that we know how
     * many significant digits are needed.
     */
    f = base->digits[0];
    p = base->weight * DEC_DIGITS;

    for (i = 1; i < base->ndigits && i * DEC_DIGITS < 16; i++)
    {
        f = f * NBASE + base->digits[i];
        p -= DEC_DIGITS;
    }

    /*----------
     * We have base ~= f * 10^p
     * so log10(result) = log10(base^exp) ~= exp * (log10(f) + p)
     *----------
     */
    f = exp * (log10(f) + p);

    /*
     * Apply crude overflow/underflow tests so we can exit early if the result
     * certainly will overflow/underflow.
     */
    if (f > 3 * SHRT_MAX * DEC_DIGITS)
        throw pq_async::exception("value overflows numeric format");
    if (f + 1 < -rscale || f + 1 < -NUMERIC_MAX_DISPLAY_SCALE)
    {
        zero_var(result);
        result->dscale = rscale;
        return;
    }

    /*
     * Approximate number of significant digits in the result.  Note that the
     * underflow test above means that this is necessarily >= 0.
     */
    sig_digits = 1 + rscale + (int) f;

    /*
     * The multiplications to produce the result may introduce an error of up
     * to around log10(abs(exp)) digits, so work with this many extra digits
     * of precision (plus a few more for good measure).
     */
    sig_digits += (int) log(std::abs(exp)) + 8;

    /*
     * Now we can proceed with the multiplications.
     */
    neg = (exp < 0);
    mask = std::abs(exp);

    //init_var(&base_prod);
    //set_var_from_var(base, &base_prod);
    base_prod = *base;

    if (mask & 1)
        //set_var_from_var(base, result);
        *result = *base;
    else
        //set_var_from_var(&const_one, result);
        *result = const_one;
    
    while ((mask >>= 1) > 0)
    {
        /*
         * Do the multiplications using rscales large enough to hold the
         * results to the required number of significant digits, but don't
         * waste time by exceeding the scales of the numbers themselves.
         */
        local_rscale = sig_digits - 2 * base_prod.weight * DEC_DIGITS;
        local_rscale = std::min(local_rscale, 2 * base_prod.dscale);
        local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

        mul_var(&base_prod, &base_prod, &base_prod, local_rscale);

        if (mask & 1)
        {
            local_rscale = sig_digits -
                (base_prod.weight + result->weight) * DEC_DIGITS;
            local_rscale = std::min(local_rscale,
                               base_prod.dscale + result->dscale);
            local_rscale = std::max(local_rscale, NUMERIC_MIN_DISPLAY_SCALE);

            mul_var(&base_prod, result, result, local_rscale);
        }

        /*
         * When abs(base) > 1, the number of digits to the left of the decimal
         * point in base_prod doubles at each iteration, so if exp is large we
         * could easily spend large amounts of time and memory space doing the
         * multiplications.  But once the weight exceeds what will fit in
         * int16, the final result is guaranteed to overflow (or underflow, if
         * exp < 0), so we can give up before wasting too many cycles.
         */
        if (base_prod.weight > SHRT_MAX || result->weight > SHRT_MAX)
        {
            /* overflow, unless neg, in which case result should be 0 */
            if (!neg)
                throw pq_async::exception("value overflows numeric format");
            zero_var(result);
            neg = false;
            break;
        }
    }

    free_var(&base_prod);

    /* Compensate for input sign, and round to requested rscale */
    if (neg)
        div_var_fast(&const_one, result, result, rscale, true);
    else
        round_var(result, rscale);
}


/* ----------------------------------------------------------------------
 *
 * Following are the lowest level functions that operate unsigned
 * on the variable level
 *
 * ----------------------------------------------------------------------
 */


/* ----------
 * cmp_abs() -
 *
 *	Compare the absolute values of var1 and var2
 *	Returns:	-1 for ABS(var1) < ABS(var2)
 *				0  for ABS(var1) == ABS(var2)
 *				1  for ABS(var1) > ABS(var2)
 * ----------
 */
int
cmp_abs(const numeric *var1, const numeric *var2)
{
    return cmp_abs_common(var1->digits, var1->ndigits, var1->weight,
                          var2->digits, var2->ndigits, var2->weight);
}


int
cmp_abs_common(
    const NumericDigit* var1digits, int var1ndigits, int var1weight,
    const NumericDigit* var2digits, int var2ndigits, int var2weight)
{
    int			i1 = 0;
    int			i2 = 0;

    /* Check any digits before the first common digit */

    while(var1weight > var2weight && i1 < var1ndigits){
        if(var1digits[i1++] != 0)
            return 1;
        var1weight--;
    }
    
    while(var2weight > var1weight && i2 < var2ndigits){
        if (var2digits[i2++] != 0)
        return -1;
        var2weight--;
    }
    
    /* At this point, either w1 == w2 or we've run out of digits */

    if(var1weight == var2weight){
        while (i1 < var1ndigits && i2 < var2ndigits){
                int			stat = var1digits[i1++] - var2digits[i2++];

            if (stat){
                if (stat > 0)
                    return 1;
                return -1;
            }
        }
    }
    
    /*
    * At this point, we've run out of digits on one side or the other;
    * so any remaining nonzero digits imply that side is larger
    */
    while (i1 < var1ndigits){
        if (var1digits[i1++] != 0)
            return 1;
    }
    
    while (i2 < var2ndigits){
        if (var2digits[i2++] != 0)
            return -1;
    }
    
    return 0;
}


/*
 * add_abs() -
 *
 *	Add the absolute values of two variables into result.
 *	result might point to one of the operands without danger.
 */
void
add_abs(const numeric *var1, const numeric *var2, numeric *result)
{
    NumericDigit *res_buf;
    NumericDigit *res_digits;
    int			res_ndigits;
    int			res_weight;
    int			res_rscale,
                rscale1,
                rscale2;
    int			res_dscale;
    int			i,
                i1,
                i2;
    int			carry = 0;

    /* copy these values into local vars for speed in inner loop */
    int			var1ndigits = var1->ndigits;
    int			var2ndigits = var2->ndigits;
    NumericDigit *var1digits = var1->digits;
    NumericDigit *var2digits = var2->digits;

    res_weight = std::max(var1->weight, var2->weight) + 1;

    res_dscale = std::max(var1->dscale, var2->dscale);

    /* Note: here we are figuring rscale in base-NBASE digits */
    rscale1 = var1->ndigits - var1->weight - 1;
    rscale2 = var2->ndigits - var2->weight - 1;
    res_rscale = std::max(rscale1, rscale2);

    res_ndigits = res_rscale + res_weight + 1;
    if (res_ndigits <= 0)
        res_ndigits = 1;

    res_buf = digitbuf_alloc(res_ndigits + 1);
    res_buf[0] = 0;				/* spare digit for later rounding */
    res_digits = res_buf + 1;

    i1 = res_rscale + var1->weight + 1;
    i2 = res_rscale + var2->weight + 1;
    for (i = res_ndigits - 1; i >= 0; i--)
    {
        i1--;
        i2--;
        if (i1 >= 0 && i1 < var1ndigits)
            carry += var1digits[i1];
        if (i2 >= 0 && i2 < var2ndigits)
            carry += var2digits[i2];

        if (carry >= NBASE)
        {
            res_digits[i] = carry - NBASE;
            carry = 1;
        }
        else
        {
            res_digits[i] = carry;
            carry = 0;
        }
    }

    assert(carry == 0);			/* else we failed to allow for carry out */

    digitbuf_free(result->buf);
    result->ndigits = res_ndigits;
    result->buf = res_buf;
    result->digits = res_digits;
    result->weight = res_weight;
    result->dscale = res_dscale;

    /* Remove leading/trailing zeroes */
    strip_var(result);
}


/*
 * sub_abs()
 *
 *	Subtract the absolute value of var2 from the absolute value of var1
 *	and store in result. result might point to one of the operands
 *	without danger.
 *
 *	ABS(var1) MUST BE GREATER OR EQUAL ABS(var2) !!!
 */
void
sub_abs(const numeric *var1, const numeric *var2, numeric *result)
{
    NumericDigit *res_buf;
    NumericDigit *res_digits;
    int			res_ndigits;
    int			res_weight;
    int			res_rscale,
                rscale1,
                rscale2;
    int			res_dscale;
    int			i,
                i1,
                i2;
    int			borrow = 0;

    /* copy these values into local vars for speed in inner loop */
    int			var1ndigits = var1->ndigits;
    int			var2ndigits = var2->ndigits;
    NumericDigit *var1digits = var1->digits;
    NumericDigit *var2digits = var2->digits;

    res_weight = var1->weight;

    res_dscale = std::max(var1->dscale, var2->dscale);

    /* Note: here we are figuring rscale in base-NBASE digits */
    rscale1 = var1->ndigits - var1->weight - 1;
    rscale2 = var2->ndigits - var2->weight - 1;
    res_rscale = std::max(rscale1, rscale2);

    res_ndigits = res_rscale + res_weight + 1;
    if (res_ndigits <= 0)
        res_ndigits = 1;

    res_buf = digitbuf_alloc(res_ndigits + 1);
    res_buf[0] = 0;				/* spare digit for later rounding */
    res_digits = res_buf + 1;

    i1 = res_rscale + var1->weight + 1;
    i2 = res_rscale + var2->weight + 1;
    for (i = res_ndigits - 1; i >= 0; i--)
    {
        i1--;
        i2--;
        if (i1 >= 0 && i1 < var1ndigits)
            borrow += var1digits[i1];
        if (i2 >= 0 && i2 < var2ndigits)
            borrow -= var2digits[i2];

        if (borrow < 0)
        {
            res_digits[i] = borrow + NBASE;
            borrow = -1;
        }
        else
        {
            res_digits[i] = borrow;
            borrow = 0;
        }
    }

    assert(borrow == 0);		/* else caller gave us var1 < var2 */

    digitbuf_free(result->buf);
    result->ndigits = res_ndigits;
    result->buf = res_buf;
    result->digits = res_digits;
    result->weight = res_weight;
    result->dscale = res_dscale;

    /* Remove leading/trailing zeroes */
    strip_var(result);
}

/*
 * round_var
 *
 * Round the value of a variable to no more than rscale decimal digits
 * after the decimal point.  NOTE: we allow rscale < 0 here, implying
 * rounding before the decimal point.
 */
void
round_var(numeric *var, int rscale)
{
    NumericDigit *digits = var->digits;
    int			di;
    int			ndigits;
    int			carry;

    var->dscale = rscale;

    /* decimal digits wanted */
    di = (var->weight + 1) * DEC_DIGITS + rscale;

    /*
     * If di = 0, the value loses all digits, but could round up to 1 if its
     * first extra digit is >= 5.  If di < 0 the result must be 0.
     */
    if (di < 0)
    {
        var->ndigits = 0;
        var->weight = 0;
        var->sign = NUMERIC_POS;
    }
    else
    {
        /* NBASE digits wanted */
        ndigits = (di + DEC_DIGITS - 1) / DEC_DIGITS;

        /* 0, or number of decimal digits to keep in last NBASE digit */
        di %= DEC_DIGITS;

        if (ndigits < var->ndigits ||
            (ndigits == var->ndigits && di > 0))
        {
            var->ndigits = ndigits;

#if DEC_DIGITS == 1
            /* di must be zero */
            carry = (digits[ndigits] >= HALF_NBASE) ? 1 : 0;
#else
            if (di == 0)
                carry = (digits[ndigits] >= HALF_NBASE) ? 1 : 0;
            else
            {
                /* Must round within last NBASE digit */
                int			extra,
                            pow10;

#if DEC_DIGITS == 4
                pow10 = round_powers[di];
#elif DEC_DIGITS == 2
                pow10 = 10;
#else
#error unsupported NBASE
#endif
                extra = digits[--ndigits] % pow10;
                digits[ndigits] -= extra;
                carry = 0;
                if (extra >= pow10 / 2)
                {
                    pow10 += digits[ndigits];
                    if (pow10 >= NBASE)
                    {
                        pow10 -= NBASE;
                        carry = 1;
                    }
                    digits[ndigits] = pow10;
                }
            }
#endif

            /* Propagate carry if needed */
            while (carry)
            {
                carry += digits[--ndigits];
                if (carry >= NBASE)
                {
                    digits[ndigits] = carry - NBASE;
                    carry = 1;
                }
                else
                {
                    digits[ndigits] = carry;
                    carry = 0;
                }
            }

            if (ndigits < 0)
            {
                assert(ndigits == -1);	/* better not have added > 1 digit */
                assert(var->digits > var->buf);
                var->digits--;
                var->ndigits++;
                var->weight++;
            }
        }
    }
}

/*
 * trunc_var
 *
 * Truncate (towards zero) the value of a variable at rscale decimal digits
 * after the decimal point.  NOTE: we allow rscale < 0 here, implying
 * truncation before the decimal point.
 */
void
trunc_var(numeric *var, int rscale)
{
    int			di;
    int			ndigits;

    var->dscale = rscale;

    /* decimal digits wanted */
    di = (var->weight + 1) * DEC_DIGITS + rscale;

    /*
     * If di <= 0, the value loses all digits.
     */
    if (di <= 0)
    {
        var->ndigits = 0;
        var->weight = 0;
        var->sign = NUMERIC_POS;
    }
    else
    {
        /* NBASE digits wanted */
        ndigits = (di + DEC_DIGITS - 1) / DEC_DIGITS;

        if (ndigits <= var->ndigits)
        {
            var->ndigits = ndigits;

#if DEC_DIGITS == 1
            /* no within-digit stuff to worry about */
#else
            /* 0, or number of decimal digits to keep in last NBASE digit */
            di %= DEC_DIGITS;

            if (di > 0)
            {
                /* Must truncate within last NBASE digit */
                NumericDigit *digits = var->digits;
                int			extra,
                            pow10;

#if DEC_DIGITS == 4
                pow10 = round_powers[di];
#elif DEC_DIGITS == 2
                pow10 = 10;
#else
#error unsupported NBASE
#endif
                extra = digits[--ndigits] % pow10;
                digits[ndigits] -= extra;
            }
#endif
        }
    }
}

/*
 * strip_var
 *
 * Strip any leading and trailing zeroes from a numeric variable
 */
void
strip_var(numeric *var)
{
    NumericDigit *digits = var->digits;
    int			ndigits = var->ndigits;

    /* Strip leading zeroes */
    while (ndigits > 0 && *digits == 0)
    {
        digits++;
        var->weight--;
        ndigits--;
    }

    /* Strip trailing zeroes */
    while (ndigits > 0 && digits[ndigits - 1] == 0)
        ndigits--;

    /* If it's zero, normalize the sign and weight */
    if (ndigits == 0)
    {
        var->sign = NUMERIC_POS;
        var->weight = 0;
    }

    var->digits = digits;
    var->ndigits = ndigits;
}



}