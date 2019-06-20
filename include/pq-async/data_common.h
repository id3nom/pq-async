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

#ifndef _libpq_async_data_common_h
#define _libpq_async_data_common_h

#include "stable_headers.h"
#include "log.h"
#include "exceptions.h"
#include "data_parameters.h"

namespace pq_async{

#define DEFAULT_CONNECTION_POOL_MAX_CONN 20

class parameters_t;
class data_value_t;
class data_column_t;
class data_columns_container_t;
class data_row_t;
class data_table_t;
class data_reader_t;
class data_large_object_t;
class data_prepared_t;

template< typename DATA_T >
class strand_t;
class database_t;

typedef std::shared_ptr< pq_async::parameters_t > parameters;
typedef std::shared_ptr< pq_async::data_table_t > data_table;
typedef std::shared_ptr< pq_async::data_row_t > data_row;
typedef std::shared_ptr<
    pq_async::data_columns_container_t 
> data_columns_container;
typedef std::shared_ptr< pq_async::data_column_t > data_column;
typedef std::shared_ptr< pq_async::data_value_t > data_value;
typedef std::shared_ptr< pq_async::data_large_object_t > data_large_object;
typedef std::shared_ptr< pq_async::data_reader_t > data_reader;
typedef std::shared_ptr< pq_async::data_prepared_t > data_prepared;


template< typename DATA_T = int >
using strand = typename std::shared_ptr< pq_async::strand_t<DATA_T> >;
typedef std::shared_ptr< pq_async::database_t > database;

///////////////////////////
// callbacks definitions //
///////////////////////////

typedef std::function<
    void(
        data_reader reader,
        const md::callback::cb_error& err,
        data_row row
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
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value && \
    !std::is_same< \
        nullptr_t, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value && \
    !std::is_array< \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value && \
    (std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type, \
        const md::callback::cb_error& \
    >::value || \
    std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type, \
        const md::callback::cb_error&, \
        _R \
    >::value || \
    std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value || \
    std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type, \
        _R \
    >::value) \
, int32_t>::type = -1

#define PQ_ASYNC_INVALID_DB_CALLBACK(_R) typename std::enable_if< \
    std::is_same< \
        _R, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value || \
    std::is_same< \
        nullptr_t, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value || \
    std::is_array< \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value || \
    (!std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type, \
        const md::callback::cb_error& \
    >::value && \
    !std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type, \
        const md::callback::cb_error&, \
        _R \
    >::value && \
    !std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
        >::type \
    >::value && \
    !std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            typename md::select_last<PARAMS...>::type \
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
        const md::callback::cb_error& \
    >::value || \
    std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            _T \
        >::type, \
        const md::callback::cb_error&, \
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
        const md::callback::cb_error& \
    >::value && \
    !std::is_invocable_r< \
        void, \
        typename std::remove_pointer< \
            _T \
        >::type, \
        const md::callback::cb_error&, \
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
        const md::callback::cb_error& \
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
        const md::callback::cb_error& \
    >::value && \
    !std::is_invocable_r< \
        void, \
        typename std::remove_pointer<_T>::type \
    >::value) \
, int32_t>::type = -1


} //namespace pq_async
#endif //_libpq_async_data_common_h
