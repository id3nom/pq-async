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

#ifndef _pq_asyncpp_range_h
#define _pq_asyncpp_range_h

#include "exceptions.h"
#include "utils.h"

#include "pg_type_numeric_def.h"
#include "pg_type_date_def.h"

namespace pq_async {

enum class range_flag
{
    none = 0x00,
    empty = 0x01, /* range is empty */
    lb_inc = 0x02, /* lower bound is inclusive */
    ub_inc = 0x04, /* upper bound is inclusive */
    lb_inf = 0x08, /* lower bound is -infinity */
    ub_inf = 0x10, /* upper bound is +infinity */
    lb_null = 0x20, /* lower bound is null (NOT USED) */
    ub_null = 0x40, /* upper bound is null (NOT USED) */
    contain_empty = 0x80 /* marks a GiST internal-page entry whose
                          * subtree contains some empty ranges
                          */
};
inline range_flag operator~(range_flag a)
{return (range_flag)(~(int)a);}
inline bool operator!(range_flag a)
{return (bool)(!(int)a);}
inline range_flag operator|(range_flag a, range_flag b)
{return (range_flag)((int)a | (int)b);}
inline range_flag operator&(range_flag a, range_flag b)
{return (range_flag)((int)a & (int)b);}
inline range_flag operator^(range_flag a, range_flag b)
{return (range_flag)((int)a ^ (int)b);}
inline range_flag& operator|=(range_flag& a, range_flag b)
{return (range_flag&)((int&)a |= (int)b);}
inline range_flag& operator&=(range_flag& a, range_flag b)
{return (range_flag&)((int&)a &= (int)b);}
inline range_flag& operator^=(range_flag& a, range_flag b)
{return (range_flag&)((int&)a ^= (int)b);}

enum class range_inclusion
{
    inclusive = 0,
    exclusive = 1,
};

class parameter;

template < typename T >
T pgval_to_range(char* val, int len, int fmt);


template<typename T>
std::ostream& operator<<(std::ostream& os, const range<T>& v);
template<typename T>
std::istream& operator>>(std::istream& is, range<T>& v);

typedef range<int32_t> int4range;
typedef range<int64_t> int8range;
typedef range<numeric> numrange;
typedef range<timestamp> tsrange;
typedef range<timestamp_tz> tstzrange;
typedef range<date> daterange;

// primary template is defined in pg_type_num_def.h
template<class T>
class range<T,
    typename std::enable_if<
        std::is_same<int32_t, T>::value ||
        std::is_same<int64_t, T>::value ||
        std::is_same<numeric, T>::value ||
        std::is_same<timestamp, T>::value ||
        std::is_same<timestamp_tz, T>::value ||
        std::is_same<date, T>::value
    >::type
>
{
    friend pq_async::parameter* new_parameter(const pq_async::int4range& value);
    friend pq_async::parameter* new_parameter(const pq_async::int8range& value);
    friend pq_async::parameter* new_parameter(const pq_async::numrange& value);
    friend pq_async::parameter* new_parameter(const pq_async::tsrange& value);
    friend pq_async::parameter* new_parameter(const pq_async::tstzrange& value);
    friend pq_async::parameter* new_parameter(const pq_async::daterange& value);

    friend pq_async::range<T> pgval_to_range< pq_async::range<T> >(
        char* val, int len, int fmt
    );
    
public:
    range()
        : _flags(range_flag::none), _lb(T()), _ub(T())
    {
    }
    
    range(T lb, T ub)
        : _flags(range_flag::lb_inc) , _lb(lb), _ub(ub)
    {
    }
    
    range(const char* str);
    operator std::string() const;
    
    bool has_lbound() const
    {
        return !(_flags & 
            (range_flag::empty | range_flag::lb_null | range_flag::lb_inf)
            );
    }
    bool has_ubound() const
    {
        return !(_flags & 
            (range_flag::empty | range_flag::ub_null | range_flag::ub_inf)
            );
    }
    bool is_empty() const
    {
        return (int)(_flags & range_flag::empty) != 0;
    }
    bool is_or_contains_empty() const
    {
        return (int)(_flags & (range_flag::empty | range_flag::contain_empty))
            != 0;
    }
    
    inline range_flag flags() const { return _flags;}
    inline range_flag& flags() { return _flags;}
    inline void flags(range_flag val){ _flags = val;}

    inline range_inclusion lbound_inclusion() const
    {
        return (int)(this->_flags & range_flag::lb_inc) != 0 ?
            range_inclusion::inclusive : range_inclusion::exclusive;
    }
    inline void lbound_inclusion(range_inclusion ri)
    {
        this->_flags &= ~(range_flag::lb_inc);
        if(ri == range_inclusion::inclusive)
            this->_flags |= range_flag::lb_inc;
    }

    inline range_inclusion ubound_inclusion() const
    {
        return (int)(this->_flags & range_flag::ub_inc) != 0 ?
            range_inclusion::inclusive : range_inclusion::exclusive;
    }
    inline void ubound_inclusion(range_inclusion ri)
    {
        this->_flags &= ~(range_flag::ub_inc);
        if(ri == range_inclusion::inclusive)
            this->_flags |= range_flag::ub_inc;
    }

    inline void clear_lbound()
    {
        if(!this->has_lbound())
            return;
        
        _flags &= ~(
            range_flag::lb_inc |
            range_flag::lb_null |
            range_flag::empty |
            range_flag::contain_empty
        );
        _flags |= range_flag::lb_inf;
        
        if(!this->has_ubound())
            _flags |= range_flag::empty;
        else
            _flags |= range_flag::contain_empty;
    }
    inline void clear_ubound()
    {
        if(!this->has_ubound())
            return;
        
        _flags &= ~(
            range_flag::ub_inc |
            range_flag::ub_null |
            range_flag::empty |
            range_flag::contain_empty
        );
        _flags |= range_flag::ub_inf;
        
        if(!this->has_lbound())
            _flags |= range_flag::empty;
        else
            _flags |= range_flag::contain_empty;
    }

    inline T lbound_abs() const
    {
        if(!this->has_lbound())
            throw pq_async::exception("lbound is unbounded");
        if(this->lbound_inclusion() == range_inclusion::inclusive)
            return _lb;
        else
            return _lb + 1;
    }
    
    inline T ubound_abs() const
    {
        if(!this->has_ubound())
            throw pq_async::exception("ubound is unbounded");
        if(this->ubound_inclusion() == range_inclusion::inclusive)
            return _ub;
        else
            return _ub - 1;
    }
    
    inline T lbound() const
    {
        if(!this->has_lbound())
            throw pq_async::exception("lbound is unbounded");
        return _lb;
    }
    inline T& lbound()
    {
        if(!this->has_lbound())
            throw pq_async::exception("lbound is unbounded");
        return _lb;
    }
    inline void lbound(T val, range_inclusion ri = range_inclusion::inclusive)
    {
        if(!this->has_lbound()){
            _flags &= ~(
                range_flag::lb_inf |
                range_flag::lb_null |
                range_flag::empty |
                range_flag::contain_empty
            );
            if(ri == range_inclusion::inclusive)
                _flags |= range_flag::lb_inc;
            if(!this->has_ubound())
                _flags |= range_flag::contain_empty;
        }
        
        _lb = val;
    }
    
    inline T ubound() const
    {
        if(!this->has_ubound())
            throw pq_async::exception("ubound is unbounded");
        return _ub;
    }
    inline T& ubound()
    {
        if(!this->has_ubound())
            throw pq_async::exception("ubound is unbounded");
        return _ub;
    }
    inline void ubound(T val, range_inclusion ri = range_inclusion::exclusive)
    {
        if(!this->has_ubound()){
            _flags &= ~(
                range_flag::ub_inf |
                range_flag::ub_null |
                range_flag::empty |
                range_flag::contain_empty
            );
            if(ri == range_inclusion::inclusive)
                _flags |= range_flag::ub_inc;
            if(!this->has_lbound())
                _flags |= range_flag::contain_empty;
        }
        
        _ub = val;
    }
    
    template<
        typename IT,
        typename ITRCV = typename std::remove_cv<T>::type
    >
    class fwd_iterator
        : public std::iterator<
            std::forward_iterator_tag,
            ITRCV,
            std::ptrdiff_t,
            IT*,
            IT&
        >
    {
        friend class range;
        
    protected:
        explicit fwd_iterator(const range<T>* r, ITRCV v)
            : _r(r), _v(v)
        {
        }
        
    public:
        void swap(fwd_iterator& other) noexcept
        {
            std::swap(_r, other._r);
            std::swap(_v, other._v);
        }
        
        IT& operator *() const
        {
            return _v;
        }
        
        IT& operator *()
        {
            return _v;
        }
        
        IT& operator ->() const
        {
            return _v;
        }
        
        IT& operator ->()
        {
            return _v;
        }

        
        const fwd_iterator &operator ++() // pre-increment
        {
            if(_r->has_ubound() && _v == _r->ubound_abs() +1)
                throw pq_async::exception("Out of bounds iterator increment!");
            ++_v;
            return *this;
        }
        
        fwd_iterator operator ++(int) // post-increment
        {
            if(_r->has_ubound() && _v == _r->ubound_abs() +1)
                throw pq_async::exception("Out of bounds iterator increment!");
            
            fwd_iterator copy(*this);
            ++_v;
            return copy;
        }
        
        template<typename OtherIteratorType>
        bool operator ==(const fwd_iterator<OtherIteratorType>& rhs) const
        {
            return _v == rhs._v;
        }
        template<typename OtherIteratorType>
        bool operator !=(const fwd_iterator<OtherIteratorType>& rhs) const
        {
            return _v != rhs._v;
        }
        
        // convert iterator to const_iterator
        operator fwd_iterator<const IT>() const
        {
            return fwd_iterator<const IT>(_r, _v);
        }
    
    private:
        const range<T>* _r;
        ITRCV _v;
    };
    
    typedef fwd_iterator<T> iterator;
    typedef fwd_iterator<const T> const_iterator;
    
    iterator begin() const
    {
        return iterator(this, this->lbound_abs());
    }
    iterator end() const
    {
        return iterator(this, this->ubound_abs() +1);
    }

    const_iterator cbegin() const
    {
        return const_iterator(this, this->lbound_abs());
    }
    const_iterator cend() const
    {
        return const_iterator(this, this->ubound_abs() +1);
    }
    
private:
    range_flag _flags;
    T _lb;
    T _ub;
};

template<typename T>
typename range<T>::iterator begin(const range<T>& v)
{
    return v.begin();
}
template<typename T>
typename range<T>::iterator end(const range<T>& v)
{
    return v.end();
}

template<typename T>
typename range<T>::const_iterator cbegin(const range<T>& v)
{
    return v.cbegin();
}
template<typename T>
typename range<T>::const_iterator cend(const range<T>& v)
{
    return v.cend();
}


template<typename T>
std::ostream& operator<<(std::ostream& os, const range<T>& v)
{
    os << (std::string)v;
    return os;
}
template<typename T>
std::istream& operator>> (std::istream& is, range<T>& v)
{
    std::string s;
    is >> s;
    v = range<T>(s.c_str());
    return is;
}


} // ns: pq_async

#endif //_pq_asyncpp_range_h
