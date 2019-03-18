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

#ifndef _libpq_async_data_row_h
#define _libpq_async_data_row_h

#define LIBPQ_ASYNC_ROW_ADD_GETTER(type, name) \
    type as_##name(const char* col_name) const \
    { \
        return get_value(col_name)->as_##name(); \
    } \
    type as_##name(int32_t i) const \
    { \
        return get_value((uint32_t)i)->as_##name(); \
    }


#include "data_common.h"
#include "data_value.h"
#include "data_column.h"
#include "data_columns_container.h"

namespace pq_async{

class data_row
{
private:
    void initialize(PGresult* row_result, int row_id);

public:
    data_row(sp_data_columns_container cols, PGresult* row_result, int row_id);

    virtual ~data_row();

    sp_data_value get_value(uint32_t i) const;
    sp_data_value get_value(const char* col_name) const;
    sp_data_value get_value(const std::string& col_name) const
    {
        return get_value(col_name.c_str());
    }

    sp_data_value operator [](uint32_t i) const
    {
        return get_value(i);
    }
    sp_data_value operator [](const char* col_name) const
    {
        return get_value(col_name);
    }
    sp_data_value operator [](const std::string& col_name) const
    {
        return get_value(col_name.c_str());
    }
    
    sp_data_value operator ()(uint32_t i) const
    {
        return get_value(i);
    }
    sp_data_value operator ()(const char* col_name) const
    {
        return get_value(col_name);
    }
    sp_data_value operator ()(const std::string& col_name) const
    {
        return get_value(col_name.c_str());
    }
    
    
    void to_json(pq_async::json& row_obj) const;
    void to_json_string(
        std::string& str, const unsigned int current_indent = 0
    ) const;
    
    sp_data_columns_container columns(){ return _cols;}
    
    /////  values
    bool is_null(int i) const
    {
        return get_value(i)->is_null();
    }

    bool is_null(const char* col_name) const
    {
        return get_value(col_name)->is_null();
    }
    
    template < typename T >
    T as(int i) const
    {
        return get_value(i)->as<T>();
    }

    template < typename T >
    T as(const char* col_name) const
    {
        return get_value(col_name)->as<T>();
    }
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(bool, bool)
    LIBPQ_ASYNC_ROW_ADD_GETTER(std::string, text)
    LIBPQ_ASYNC_ROW_ADD_GETTER(int16_t, int16)
    LIBPQ_ASYNC_ROW_ADD_GETTER(int32_t, int32)
    LIBPQ_ASYNC_ROW_ADD_GETTER(int64_t, int64)
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::numeric, numeric)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::money, money)

    LIBPQ_ASYNC_ROW_ADD_GETTER(float, float)
    LIBPQ_ASYNC_ROW_ADD_GETTER(double, double)
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::time, time)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::time_tz, time_tz)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::timestamp, timestamp)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::timestamp_tz, timestamp_tz)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::date, date)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::interval, interval)
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::json, json)
    LIBPQ_ASYNC_ROW_ADD_GETTER(std::vector<int8_t>, bytea)
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::uuid, uuid)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::oid, oid)

    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::cidr, cidr)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::inet, inet)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::macaddr, macaddr)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::macaddr8, macaddr8)
    
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::point, point)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::line, line)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::lseg, lseg)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::box, box)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::path, path)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::polygon, polygon)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::circle, circle)

    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::int4range, int4range)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::int8range, int8range)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::numrange, numrange)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::tsrange, tsrange)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::tstzrange, tstzrange)
    LIBPQ_ASYNC_ROW_ADD_GETTER(pq_async::daterange, daterange)
    
    
    bool is_array(const char* col_name) const
    {
        return get_value(col_name)->is_array();
    }

    int get_array_ele_oid(const char* col_name) const
    {
        return get_value(col_name)->get_array_ele_oid();
    }

    int get_array_dim(const char* col_name) const
    {
        return get_value(col_name)->get_array_dim();
    }

    template<class T>
    md::jagged_vector<T> as_array(const char* col_name) const
    {
        return get_value(col_name)->as_array<T>();
    }


    bool is_array(uint32_t i) const
    {
        return get_value(i)->is_array();
    }

    int get_array_ele_oid(uint32_t i) const
    {
        return get_value(i)->get_array_ele_oid();
    }

    int get_array_dim(uint32_t i) const
    {
        return get_value(i)->get_array_dim();
    }

    template<class T>
    md::jagged_vector<T> as_array(uint32_t i) const
    {
        return get_value(i)->as_array<T>();
    }

private:
    //static int32_t val_count;

    //std::shared_ptr< data_table > _table;
    //sp_data_table _table;
    sp_data_columns_container _cols;
    std::vector< sp_data_value > _values;
};

#undef LIBPQ_ASYNC_ROW_ADD_GETTER

} //namespace pq_async
#endif //_libpq_async_data_row_h
