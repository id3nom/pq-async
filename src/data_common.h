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

#ifndef _libpq_async_data_common_h
#define _libpq_async_data_common_h

#include "stable_headers.h"
#include "log.h"
#include "exceptions.h"
#include "data_parameters.h"
//#include "delegate.h"
#include "event_queue.h"
#include "event_strand.h"

#include "async.h"

namespace pq_async{

#define DEFAULT_CONNECTION_POOL_MAX_CONN 20

class parameters;
class data_value;
class data_column;
class data_columns_container;
class data_row;
class data_table;
class data_reader;
class data_large_object;
class data_prepared;

template< typename DATA_T >
class strand;
class database;

typedef std::shared_ptr< pq_async::parameters > sp_parameters;
typedef std::shared_ptr< pq_async::data_table > sp_data_table;
typedef std::shared_ptr< pq_async::data_row > sp_data_row;
typedef std::shared_ptr< pq_async::data_columns_container > sp_data_columns_container;
typedef std::shared_ptr< pq_async::data_column > sp_data_column;
typedef std::shared_ptr< pq_async::data_value > sp_data_value;
typedef std::shared_ptr< pq_async::data_large_object > sp_data_large_object;
typedef std::shared_ptr< pq_async::data_reader > sp_data_reader;
typedef std::shared_ptr< pq_async::data_prepared > sp_data_prepared;


template< typename DATA_T = int >
using sp_strand = typename std::shared_ptr< pq_async::strand<DATA_T> >;
typedef std::shared_ptr< pq_async::database > sp_database;

///////////////////////////
// callbacks definitions //
///////////////////////////

typedef std::function<
	void(
		sp_data_reader reader,
		const pq_async::cb_error& err,
		sp_data_row row
	)
> query_next_cb;


enum class data_type
{
	// bytea type
	bytea = BYTEAOID,
	bytea_arr = BYTEAARRAYOID,
	
	// bool type
	boolean = BOOLOID,
	boolean_arr = BOOLARRAYOID,
	
	// text types
	text = TEXTOID,
	text_arr = TEXTARRAYOID,
	varchar = data_type::text,
	varchar_arr = data_type::text_arr,
	chr = data_type::text,
	chr_arr = data_type::text_arr,
	
	// date and time types
	timestamp = TIMESTAMPOID,
	timestamp_arr = TIMESTAMPARRAYOID,
	timestamp_tz = TIMESTAMPTZOID,
	timestamp_tz_arr = TIMESTAMPTZARRAYOID,
	date = DATEOID,
	date_arr = DATEARRAYOID,
	time = TIMEOID,
	time_arr = TIMEARRAYOID,
	time_tz = TIMETZOID,
	time_tz_arr = TIMETZARRAYOID,
	interval = INTERVALOID,
	interval_arr = INTERVALARRAYOID,
	
	// geo types
	point = POINTOID,
	point_arr = POINTARRAYOID,
	line = LINEOID,
	line_arr = LINEARRAYOID,
	lseg = LSEGOID,
	lseg_arr = LSEGARRAYOID,
	box = BOXOID,
	box_arr = BOXARRAYOID,
	path = PATHOID,
	path_arr = PATHARRAYOID,
	polygon = POLYGONOID,
	polygon_arr = POLYGONARRAYOID,
	circle = CIRCLEOID,
	circle_arr = CIRCLEARRAYOID,
	
	// json types
	json = JSONOID,
	json_arr = JSONARRAYOID,
	jsonb = data_type::json,
	jsonb_arr = data_type::json_arr,
	
	// money type
	money = CASHOID,
	money_arr = MONEYARRAYOID,
	
	// network types
	cidr = CIDROID,
	cidr_arr = CIDRARRAYOID,
	inet = INETOID,
	inet_arr = INETARRAYOID,
	macaddr = MACADDROID,
	macaddr_arr = MACADDRARRAYOID,
	macaddr8 = MACADDR8OID,
	macaddr8_arr = MACADDR8ARRAYOID,
	
	// numeric types
	smallint = INT2OID,
	smallint_arr = INT2ARRAYOID,
	integer = INT4OID,
	integer_arr = INT4ARRAYOID,
	bigint = INT8OID,
	bigint_arr = INT8ARRAYOID,
	numeric = NUMERICOID,
	numeric_arr = NUMERICARRAYOID,

	real = FLOAT4OID,
	real_arr = FLOAT4ARRAYOID,
	float4 = FLOAT4OID,
	float4_arr = FLOAT4ARRAYOID,
	dble = FLOAT8OID,
	dble_arr = FLOAT8ARRAYOID,
	float8 = FLOAT8OID,
	float8_arr = FLOAT8ARRAYOID,
	
	// range types
	int4range = INT4RANGEOID,
	int4range_arr = INT4RANGEARRAYOID,
	int8range = INT8RANGEOID,
	int8range_arr = INT8RANGEARRAYOID,
	numrange = NUMRANGEOID,
	numrange_arr = NUMRANGEARRAYOID,
	tsrange = TSRANGEOID,
	tsrange_arr = TSRANGEARRAYOID,
	tstzrange = TSTZRANGEOID,
	tstzrange_arr = TSTZRANGEARRAYOID,
	daterange = DATERANGEOID,
	daterange_arr = DATERANGEARRAYOID,
	
	// system type
	oid = OIDOID,
	oid_arr = OIDARRAYOID,
	
	// uuid type
	uuid = UUIDOID,
	uuid_arr = UUIDARRAYOID,
	
};

#define PQ_ASYNC_VALID_DB_CALLBACK(_R) typename std::enable_if< \
	!std::is_same< \
		_R, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value && \
	!std::is_array< \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value && \
	(std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		const cb_error& \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		const cb_error&, \
		_R \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		_R \
	>::value) \
, int32_t>::type = -1

#define PQ_ASYNC_INVALID_DB_CALLBACK(_R) typename std::enable_if< \
	std::is_same< \
		_R, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value || \
	std::is_array< \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value || \
	(!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		const cb_error& \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		const cb_error&, \
		_R \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			typename select_last<PARAMS...>::type \
		>::type, \
		_R \
	>::value) \
, int32_t>::type = -1

#define PQ_ASYNC_VALID_DB_VAL_CALLBACK(_T, _R) typename std::enable_if< \
	!std::is_array< \
		typename std::remove_pointer< \
			_T \
		>::type \
	>::value && \
	(std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		const cb_error& \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		const cb_error&, \
		_R \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		_R \
	>::value) \
, int32_t>::type = -1

#define PQ_ASYNC_INVALID_DB_VAL_CALLBACK(_T, _R) typename std::enable_if< \
	std::is_array< \
		typename std::remove_pointer< \
			_T \
		>::type \
	>::value || \
	(!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		const cb_error& \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		const cb_error&, \
		_R \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer< \
			_T \
		>::type, \
		_R \
	>::value) \
, int32_t>::type = -1


#define PQ_ASYNC_VALID_DB_ASY_CALLBACK(_T) typename std::enable_if< \
	!std::is_array< \
		typename std::remove_pointer<_T>::type \
	>::value && \
	(std::is_invocable_r< \
		void, \
		typename std::remove_pointer<_T>::type, \
		const cb_error& \
	>::value || \
	std::is_invocable_r< \
		void, \
		typename std::remove_pointer<_T>::type \
	>::value) \
, int32_t>::type = -1

#define PQ_ASYNC_INVALID_DB_ASY_CALLBACK(_T) typename std::enable_if< \
	std::is_array< \
		typename std::remove_pointer<_T>::type \
	>::value || \
	(!std::is_invocable_r< \
		void, \
		typename std::remove_pointer<_T>::type, \
		const cb_error& \
	>::value && \
	!std::is_invocable_r< \
		void, \
		typename std::remove_pointer<_T>::type \
	>::value) \
, int32_t>::type = -1


} //namespace pq_async
#endif //_libpq_async_data_common_h
