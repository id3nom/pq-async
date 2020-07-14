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

#include "data_row.h"

namespace pq_async{

data_row_t::data_row_t(
    data_columns_container cols, 
    PGresult* row_result, int row_id)
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);

    _cols = cols;
    initialize(row_result, row_id);
}

data_row_t::~data_row_t()
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
}


void data_row_t::initialize(PGresult* row_result, int row_id)
{
    for(size_t i = 0; i < _cols->size(); ++i){
        data_column col = _cols->get_col(i);
        
        int isNull = PQgetisnull(row_result, row_id, i);
        
        if(isNull)
            _values.emplace_back(data_value(new data_value_t(col, NULL, 0)));
        else{
            char* pgValues = PQgetvalue(row_result, row_id, i);
            int len = PQgetlength(row_result, row_id, i);

            char* values = new char[len];
            memcpy(values, pgValues, len);

            _values.emplace_back(data_value(new data_value_t(col, values, len)));
        }
    }
}

data_value data_row_t::get_value(uint32_t i) const
{
    if(i >= _cols->size())
        throw pq_async::exception("Invalid index.");

    return _values[i];
}

data_value data_row_t::get_value(
    const char* col_name) const
{
    int index = _cols->get_col_index(col_name);
    if(index == -1){
        std::string msg("Column name \"");
        msg.append(col_name);
        msg.append("\" is not valid.");
        throw pq_async::exception(msg.c_str());
    }
    return _values[index];
}




#define _PQ_ASYNC_ARRAY_TO_JSON(DIM_TYPE, __arr_cast) \
if(_values[i]->is_null()) \
    row_obj[name] = nullptr; \
else { \
    md::jagged_vector<DIM_TYPE> jv = _values[i]->as_array<DIM_TYPE>(); \
    pq_async::json rows_arr = pq_async::json::array(); \
    for(size_t d = 0; d < jv.dim_size(); ++d){ \
        pq_async::json row_arr = pq_async::json::array(); \
        for(auto it = jv.cbegin(d); it != jv.cend(d); ++it){ \
            row_arr.push_back((__arr_cast)(*it)); \
        } \
        if(jv.dim_size() == 1) \
            rows_arr = row_arr; \
        else \
            rows_arr.push_back(row_arr); \
    } \
    row_obj[name] = rows_arr; \
}

#define _PQ_ASYNC_TO_JSON(__oid, __cast, __arr_oid, __arr_type, __arr_cast) \
case __oid: \
    row_obj[name] = _values[i]->as_##__cast(); \
    break; \
case __arr_oid: \
    _PQ_ASYNC_ARRAY_TO_JSON(__arr_type, __arr_cast) \
    break;

void data_row_t::to_json(pq_async::json& row_obj) const
{
    for(unsigned int i = 0; i < _values.size(); ++i){
        const char* name = _values[i]->column()->get_cname();
        int oid = _values[i]->column()->get_oid();
        
        if(_values[i]->is_null())
            row_obj[name] = nullptr;
        else
            switch(oid){
                _PQ_ASYNC_TO_JSON(
                    BOOLOID, bool, BOOLARRAYOID, bool, bool
                )
                _PQ_ASYNC_TO_JSON(
                    TEXTOID, text, 
                    TEXTARRAYOID, std::string, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    INT2OID, int16,
                    INT2ARRAYOID, int16_t, int16_t
                )
                _PQ_ASYNC_TO_JSON(
                    INT4OID, int32,
                    INT4ARRAYOID, int32_t, int32_t
                )
                _PQ_ASYNC_TO_JSON(
                    INT8OID, int64,
                    INT8ARRAYOID, int64_t, int64_t
                )
                
                _PQ_ASYNC_TO_JSON(
                    NUMERICOID, numeric,
                    NUMERICARRAYOID, pq_async::numeric, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    CASHOID, money,
                    MONEYARRAYOID, pq_async::money, std::string
                )

                _PQ_ASYNC_TO_JSON(
                    FLOAT4OID, float, FLOAT4ARRAYOID, float, float
                )
                _PQ_ASYNC_TO_JSON(
                    FLOAT8OID, double,
                    FLOAT8ARRAYOID, double, double
                )
                
                _PQ_ASYNC_TO_JSON(
                    TIMEOID, time,
                    TIMEARRAYOID, pq_async::time, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    TIMETZOID, time_tz,
                    TIMETZARRAYOID, pq_async::time_tz, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    TIMESTAMPOID, timestamp,
                    TIMESTAMPARRAYOID, pq_async::timestamp, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    TIMESTAMPTZOID, timestamp_tz,
                    TIMESTAMPTZARRAYOID, pq_async::timestamp_tz, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    DATEOID, date,
                    DATEARRAYOID, pq_async::date, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    INTERVALOID, interval,
                    INTERVALARRAYOID, pq_async::interval, std::string
                )
                
                _PQ_ASYNC_TO_JSON(
                    JSONOID, json,
                    JSONARRAYOID, pq_async::json, pq_async::json
                )
                _PQ_ASYNC_TO_JSON(
                    JSONBOID, json,
                    JSONBARRAYOID, pq_async::json, pq_async::json
                )
                
                _PQ_ASYNC_TO_JSON(
                    BYTEAOID, bytea,
                    BYTEAARRAYOID, std::vector<int8_t>, std::vector<int8_t>
                )
                
                _PQ_ASYNC_TO_JSON(
                    UUIDOID, uuid,
                    UUIDARRAYOID, pq_async::uuid, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    OIDOID, oid,
                    OIDARRAYOID, pq_async::oid, int64_t
                )

                _PQ_ASYNC_TO_JSON(
                    CIDROID, cidr,
                    CIDARRAYOID, pq_async::cidr, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    INETOID, inet,
                    INETARRAYOID, pq_async::inet, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    MACADDROID, macaddr,
                    MACADDRARRAYOID, pq_async::macaddr, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    MACADDR8OID, macaddr8,
                    MACADDR8ARRAYOID, pq_async::macaddr8, std::string
                )
                
                _PQ_ASYNC_TO_JSON(
                    POINTOID, point,
                    POINTARRAYOID, pq_async::point, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    LINEOID, line,
                    LINEARRAYOID, pq_async::line, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    LSEGOID, lseg,
                    LSEGARRAYOID, pq_async::lseg, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    BOXOID, box,
                    BOXARRAYOID, pq_async::box, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    PATHOID, path,
                    PATHARRAYOID, pq_async::path, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    POLYGONOID, polygon,
                    POLYGONARRAYOID, pq_async::polygon, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    CIRCLEOID, circle,
                    CIRCLEARRAYOID, pq_async::circle, std::string
                )

                _PQ_ASYNC_TO_JSON(
                    INT4RANGEOID, int4range,
                    INT4RANGEARRAYOID, pq_async::int4range, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    INT8RANGEOID, int8range,
                    INT8RANGEARRAYOID, pq_async::int8range, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    NUMRANGEOID, numrange,
                    NUMRANGEARRAYOID, pq_async::numeric, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    TSRANGEOID, tsrange,
                    TSRANGEARRAYOID, pq_async::tsrange, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    TSTZRANGEOID, tstzrange,
                    TSTZRANGEARRAYOID, pq_async::tstzrange, std::string
                )
                _PQ_ASYNC_TO_JSON(
                    DATERANGEOID, daterange,
                    DATERANGEARRAYOID, pq_async::daterange, std::string
                )

                default:
                    pq_async::default_logger()->warn(
                        "Unsupported OID: {} for field: {}",
                        oid, name
                    );
                    row_obj[name] = 
                        _values[i]->is_null() ? 
                        nullptr : _values[i]->as_text();
                    break;
            }
    }
}


void data_row_t::to_json_string(
    std::string& str, const unsigned int current_indent) const
{
    pq_async::json row = pq_async::json::object();
    to_json(row);
    str.append(row.dump(current_indent));
}


} //namespace pq_async