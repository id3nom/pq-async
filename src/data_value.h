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

class data_value;
typedef std::shared_ptr< pq_async::data_value > sp_data_value;

class data_value
{
public:
    data_value(sp_data_column col, char* value, int length);
    
    virtual ~data_value();
    
    sp_data_column column(){ return _col;}
    
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

    template<class T, int32_t dim_count>
    operator boost::multi_array<T, dim_count>() const
    {
        return as_array<T, dim_count>();
    }
    
    template<class T, int32_t dim_count>
    boost::multi_array<T, dim_count> as_array() const
    {
        return pgval_to_array<T, dim_count>(
            (char*)_value, _length, _col->get_format()
            );
    }

private:
    sp_data_column _col;
    char* _value;
    int _length;

    int _ele_oid;
    int _dim;
};

#undef LIBPQ_ASYNC_VAL_ADD_GETTER

} //namespace pq_async
#endif //_libpq_async_data_value_h
