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

#include "data_row.h"

namespace pq_async{

data_row::data_row(
	sp_data_columns_container cols, 
	PGresult* row_result, int row_id)
{
	pq_async_log_trace("ptr: %p", this);

	_cols = cols;
	initialize(row_result, row_id);
}

data_row::~data_row()
{
	pq_async_log_trace("ptr: %p", this);
}


void data_row::initialize(PGresult* row_result, int row_id)
{
	for(size_t i = 0; i < _cols->size(); ++i){
		sp_data_column col = _cols->get_col(i);
		
		int isNull = PQgetisnull(row_result, row_id, i);
		
		if(isNull)
			_values.emplace_back(sp_data_value(new data_value(col, NULL, 0)));
		else{
			char* pgValues = PQgetvalue(row_result, row_id, i);
			int len = PQgetlength(row_result, row_id, i);

			char* values = new char[len];
			memcpy(values, pgValues, len);

			_values.emplace_back(sp_data_value(new data_value(col, values, len)));
		}
	}
}

sp_data_value data_row::get_value(uint32_t i) const
{
	if(i >= _cols->size())
		throw pq_async::exception("Invalid index.");

	return _values[i];
}

sp_data_value data_row::get_value(
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



#define _PQ_ASYNC_ARRAY_DIM_TO_JSON(DIM_TYPE, DIM_SIZE, __arr_cast) \
} else if(dim == DIM_SIZE) { \
	boost::multi_array< DIM_TYPE, DIM_SIZE > data_arr =  \
		_values[i]->as_array< DIM_TYPE, DIM_SIZE >(); \
 \
	auto itval = data_arr.data(); \
	for(unsigned int j = 0; j < DIM_SIZE; ++j){ \
		pq_async::json row_arr = pq_async::json::array(); \
		uint32_t dim_ele_cnt = data_arr.shape()[i]; \
		for(unsigned int k = 0; k < dim_ele_cnt; ++k){ \
			DIM_TYPE v = *itval; \
			row_arr.push_back((__arr_cast)v); \
			++itval; \
		} \
		rows_arr.push_back(row_arr); \
	}

#define _PQ_ASYNC_ARRAY_TO_JSON(DIM_TYPE, __arr_cast) \
if(_values[i]->is_null()) \
	row_obj[name] = nullptr; \
else { \
	int dim = _values[i]->get_array_dim(); \
	pq_async::json rows_arr = pq_async::json::array(); \
	\
	if(dim == 1){ \
		boost::multi_array< DIM_TYPE, 1 > data_arr =  \
			_values[i]->as_array< DIM_TYPE, 1 >(); \
	\
		for(unsigned int j = 0; j < data_arr.shape()[0]; ++j){ \
			DIM_TYPE v = data_arr[j]; \
			rows_arr.push_back((__arr_cast)v); \
		} \
	\
	_PQ_ASYNC_ARRAY_DIM_TO_JSON(DIM_TYPE, 2, __arr_cast) \
	_PQ_ASYNC_ARRAY_DIM_TO_JSON(DIM_TYPE, 3, __arr_cast) \
	_PQ_ASYNC_ARRAY_DIM_TO_JSON(DIM_TYPE, 4, __arr_cast) \
	_PQ_ASYNC_ARRAY_DIM_TO_JSON(DIM_TYPE, 5, __arr_cast) \
	\
	} else { \
		throw pq_async::exception( \
			"6 Dimension and higher are not supported!." \
		); \
	} \
	\
	row_obj[name] = rows_arr; \
}

#define _PQ_ASYNC_TO_JSON(__oid, __cast, __arr_oid, __arr_type, __arr_cast) \
case __oid: \
	row_obj[name] = _values[i]->as_##__cast(); \
	break; \
case __arr_oid: \
	_PQ_ASYNC_ARRAY_TO_JSON(__arr_type, __arr_cast) \
	break;

void data_row::to_json(pq_async::json& row_obj) const
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
					pq_async_log_trace(
						"Unsupported OID: %d for field: %s",
						oid, name
					);
					row_obj[name] = 
						_values[i]->is_null() ? 
						nullptr : _values[i]->as_text();
					break;
			}
	}
}


void data_row::to_json_string(
	std::string& str, const unsigned int current_indent) const
{
	pq_async::json row = pq_async::json::object();
	to_json(row);
	str.append(row.dump(current_indent));
}


} //namespace pq_async