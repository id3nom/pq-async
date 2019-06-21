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

#ifndef _libpq_async_data_value_h
#define _libpq_async_data_value_h

#define LIBPQ_ASYNC_VAL_ADD_GETTER(type, name) operator type() const \
    { \
        return as_##name(); \
    } \
    type as_##name() const \
    { \
        return val_from_pgparam<type>( \
            _col->get_oid(), _value, _length, _col->get_format() \
            ); \
    }

#include "data_common.h"
#include "data_column.h"

namespace pq_async{

class data_value_t;
typedef std::shared_ptr< pq_async::data_value_t > data_value;

class data_value_t
{
public:
    data_value_t(data_column col, char* value, int length);
    
    virtual ~data_value_t();
    
    data_column column(){ return _col;}
    
    bool is_null(){ return _value == NULL;}
    
    template < typename T >
    T as()
    {
        return val_from_pgparam<T>(
            _col->get_oid(), _value, _length, _col->get_format()
            );
    }
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(bool, bool)
    LIBPQ_ASYNC_VAL_ADD_GETTER(std::string, text)
    LIBPQ_ASYNC_VAL_ADD_GETTER(int16_t, int16)
    LIBPQ_ASYNC_VAL_ADD_GETTER(int32_t, int32)
    LIBPQ_ASYNC_VAL_ADD_GETTER(int64_t, int64)
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::numeric, numeric)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::money, money)

    LIBPQ_ASYNC_VAL_ADD_GETTER(float, float)
    LIBPQ_ASYNC_VAL_ADD_GETTER(double, double)
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::time, time)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::time_tz, time_tz)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::timestamp, timestamp)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::timestamp_tz, timestamp_tz)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::date, date)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::interval, interval)
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::json, json)
    LIBPQ_ASYNC_VAL_ADD_GETTER(std::vector<int8_t>, bytea)
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::uuid, uuid)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::oid, oid)

    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::cidr, cidr)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::inet, inet)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::macaddr, macaddr)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::macaddr8, macaddr8)
    
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::point, point)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::line, line)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::lseg, lseg)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::box, box)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::path, path)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::polygon, polygon)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::circle, circle)

    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::int4range, int4range)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::int8range, int8range)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::numrange, numrange)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::tsrange, tsrange)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::tstzrange, tstzrange)
    LIBPQ_ASYNC_VAL_ADD_GETTER(pq_async::daterange, daterange)
    
    bool is_array()
    {
        int oid = _col->get_oid();
        switch(oid){
            case UUIDARRAYOID:
            case CSTRINGARRAYOID:
            case REGTYPEARRAYOID:
            case RECORDARRAYOID:
            case ANYARRAYOID:
            case ANYNONARRAYOID:
            case INT4ARRAYOID:
            case TEXTARRAYOID:
            case FLOAT4ARRAYOID:
            case BOOLARRAYOID:
            case INT2ARRAYOID:
            case INT8ARRAYOID:
            case FLOAT8ARRAYOID:
            case NUMERICARRAYOID:
            case TIMESTAMPARRAYOID:
            case TIMESTAMPTZARRAYOID:
            case TIMEARRAYOID:
            case TIMETZARRAYOID:
            case JSONARRAYOID:
            case JSONBARRAYOID:
            case BYTEAARRAYOID:
            case INTERVALARRAYOID:
            case MONEYARRAYOID:
            case CHARARRAYOID:
            case XIDARRAYOID:
            case CIDARRAYOID:
            case BPCHARARRAYOID:
            case VARCHARARRAYOID:
            case ABSTIMEARRAYOID:
            case RELTIMEARRAYOID:
            case OIDARRAYOID:
            case CIDRARRAYOID:
            case INETARRAYOID:
            case MACADDRARRAYOID:
            case MACADDR8ARRAYOID:
            case POINTARRAYOID:
            case LINEARRAYOID:
            case LSEGARRAYOID:
            case BOXARRAYOID:
            case PATHARRAYOID:
            case POLYGONARRAYOID:
            case CIRCLEARRAYOID:
            case INT4RANGEARRAYOID:
            case INT8RANGEARRAYOID:
            case NUMRANGEARRAYOID:
            case TSRANGEARRAYOID:
            case TSTZRANGEARRAYOID:
            case DATERANGEARRAYOID:
            
                return true;

            default:
                return false;
        }
    }

    int get_array_ele_oid()
    {
        if(_ele_oid != -1 && _dim != -1)
            return _ele_oid;

        get_array_oid_and_dim(
            (char*)_value, _length, _col->get_format(), _ele_oid, _dim
            );

        return _ele_oid;
    }

    int get_array_dim()
    {
        if(_ele_oid != -1 && _dim != -1)
            return _dim;

        get_array_oid_and_dim(
            (char*)_value, _length, _col->get_format(), _ele_oid, _dim
            );

        return _dim;
    }

    template<class T>
    operator md::jagged_vector<T>() const
    {
        return as_array<T>();
    }
    
    template<class T>
    md::jagged_vector<T> as_array() const
    {
        return pgval_to_array<T>(
            (char*)_value, _length, _col->get_format()
            );
    }

private:
    data_column _col;
    char* _value;
    int _length;

    int _ele_oid;
    int _dim;
};

#undef LIBPQ_ASYNC_VAL_ADD_GETTER

} //namespace pq_async
#endif //_libpq_async_data_value_h
