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

#include <iostream>
#include <iomanip>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_parameters.h"

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef POSTGRES_EPOCH_JDATE
#define POSTGRES_EPOCH_JDATE 10957
#endif

#ifndef POSTGRES_EPOCH_USEC
#define POSTGRES_EPOCH_USEC 946684800000000LL
#endif

/*
 * We use these values for the "family" field.
 *
 * Referencing all of the non-AF_INET types to AF_INET lets us work on
 * machines which may not have the appropriate address family (like
 * inet6 addresses when AF_INET6 isn't present) but doesn't cause a
 * dump/reload requirement.  Pre-7.4 databases used AF_INET for the family
 * type on disk.
 */
#define PGSQL_AF_INET	(AF_INET + 0)
#define PGSQL_AF_INET6	(AF_INET + 1)


namespace pq_async{
namespace hhdate = date;


/// parameters code.

bool pgval_to_bool(char* val, int len, int fmt)
{
    if(len == -1)
        return false;

    if(!fmt){
        std::string str1(val, len);
        return
            pq_async::iequals(str1, "true") ||
            pq_async::iequals(str1, "yes") ||
            pq_async::iequals(str1, "on") ||
            pq_async::iequals(str1, "1");
    }

    // else result is in bin format.
    return val[0] != 0;// std::string(val, val + len);
}

std::string pgval_to_string(char* val, int len, int fmt)
{
    if(len == -1)
        return "";

    if(!fmt)// if result in text format.
        return std::string(val, len);
    return std::string(val, len);
}

int16_t pgval_to_int16(char* val, int len, int fmt)
{
    if(len == -1)
        return 0;

    if(!fmt)
        return pq_async::str_to_num<int16_t>(std::string(val, len));

    int16_t value = 0;// *((int16_t*)val);
    pq_async::swap2((int16_t*)val, &value, false);

    return value;
}

int32_t pgval_to_int32(char* val, int len, int fmt)
{
    if(len == -1)
        return 0;

    if(!fmt)
        return pq_async::str_to_num<int32_t>(std::string(val, len));

    int32_t value = 0; //*((int32_t*)val);
    pq_async::swap4((int32_t*)val, &value, false);

    return value;
}

int64_t pgval_to_int64(char* val, int len, int fmt)
{
    if(len == -1)
        return 0;

    if(!fmt)
        return pq_async::str_to_num<int64_t>(std::string(val, len));

    int64_t value = 0;// *((int64_t*)val);
    pq_async::swap8((int64_t*)val, &value, false);

    return value;
}

pq_async::numeric pgval_to_numeric(char* val, int len, int fmt)
{
    if(len == -1) {
        return pq_async::numeric("0", 1);
    }

    if(!fmt) {
        return pq_async::numeric(val, len);
    }
    
    pq_async::numeric result;
    
    int16_t* s = (int16_t *)val;
    int16_t out_val = 0;
    pq_async::swap2((int16_t*)s, &out_val, false); ++s;
    result.ndigits = out_val;
    pq_async::swap2((int16_t*)s, &out_val, false); ++s;
    result.weight = out_val;
    pq_async::swap2((int16_t*)s, &out_val, false); ++s;
    result.sign = out_val;
    pq_async::swap2((int16_t*)s, &out_val, false); ++s;
    result.dscale = out_val;

    result.buf = new int16_t[result.ndigits +1];
    result.buf[0] = 0;
    result.digits = result.buf +1;
    for(int i = 0; i < result.ndigits; ++i) {
        int16_t dval = 0;
        pq_async::swap2((int16_t*)s, &dval, false); ++s;
        result.digits[i] = dval;
    }
    
    // std::cout << "numeric put, len: " << (int)len << std::endl << "values: ";
    // for(size_t i = 0; i < (size_t)len; ++i)
    // 	std::cout << std::hex << (int)val[i] << ", ";
    // std::cout << std::endl;
    // std::cout << std::endl;
    
    return result;
}

pq_async::money pgval_to_money(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::money();

    pq_async::money m;
    if(!fmt){
        m._val = pq_async::str_to_num<int64_t>(std::string(val, len));
        return m;
    }
    int64_t value = 0;// *((int64_t*)val);
    pq_async::swap8((int64_t*)val, &value, false);
    m._val = value;
    return m;
}

float pgval_to_float(char* val, int len, int fmt)
{
    if(len == -1)
        return 0.0f;

    if(!fmt)
        return pq_async::str_to_num<float>(std::string(val, len));

    float value = 0.0f; //*((int32_t*)val);
    pq_async::swap4((int32_t*)val, (int32_t*)&value, false);

    return value;
}

double pgval_to_double(char* val, int len, int fmt)
{
    if(len == -1)
        return 0.0;

    if(!fmt)
        return pq_async::str_to_num<double>(std::string(val, len));

    double value = 0.0;// *((int64_t*)val);
    pq_async::swap8((int64_t*)val, (int64_t*)&value, false);

    return value;
}


pq_async::time pgval_to_time(char* val, int len, int fmt)
{
    if(len == -1)
        return time(0);

    if(!fmt)
        throw pq_async::exception("text result format not supported for time!");

    int64_t tval = 0;
    swap8((int64_t*)val, &tval, false);

    tval += POSTGRES_EPOCH_USEC;

    return pq_async::time(tval);
}

pq_async::time_tz pgval_to_time_tz(char* val, int len, int fmt)
{
    if(len == -1)
        return time_tz(hhdate::locate_zone("UTC"), 0);

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for time with time zone!"
        );

    int64_t tval = 0;
    swap8((int64_t*)val, &tval, false);
    
    // zone-name;
    // int32_t zn = 0;
    // swap4((int32_t*)(val +sizeof(int64_t)), &zn, false);
    // std::string z((char*)&zn, 4);
    // std::cout << z << std::endl;
    
    tval += POSTGRES_EPOCH_USEC;

    return pq_async::time_tz(hhdate::locate_zone("UTC"), tval);
}

pq_async::timestamp pgval_to_timestamp(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::timestamp();

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for timetamp!"
        );

    int64_t tval = 0;
    swap8((int64_t*)val, &tval, false);

    tval += POSTGRES_EPOCH_USEC;

    return pq_async::timestamp(tval);
}

pq_async::timestamp_tz pgval_to_timestamp_tz(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::timestamp_tz();

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for timetamp with time zone!"
        );

    int64_t tval = 0;
    swap8((int64_t*)val, &tval, false);

    tval += POSTGRES_EPOCH_USEC;

    return pq_async::timestamp_tz(tval);
}

pq_async::date pgval_to_date(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::date();

    //TODO: implement date parsing
    if(!fmt)
        throw pq_async::exception("text result format not supported for date!");

    int32_t dval = 0;
    swap4((int32_t*)val, &dval, false);

    dval += POSTGRES_EPOCH_JDATE;

    return pq_async::date(dval);
}

pq_async::interval pgval_to_interval(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::interval();

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for interval!"
        );

    int64_t tval = 0;
    swap8((int64_t*)val, &tval, false);
    int32_t dval = 0;
    swap4((int32_t*)(val +8), &dval, false);
    int32_t mval = 0;
    swap4((int32_t*)(val +12), &mval, false);

    return pq_async::interval(tval, dval, mval);
}

std::vector<int8_t> pgval_to_bytea(char* val, int len, int fmt)
{
    if(len == -1)
        return std::vector<int8_t>();

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for bytea!"
        );

    int8_t* start = (int8_t*)val;
    std::vector<int8_t> bytea((size_t)len);
    bytea.assign(start, start +len);
    return bytea;
}

pq_async::uuid pgval_to_uuid(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::uuid();

    if(!fmt)
        throw pq_async::exception(
            "text result format not supported for uuid!"
        );

    if(len != 16)
        throw pq_async::exception(
            "len is not valid for uuid type!"
        );

    return pq_async::uuid(reinterpret_cast<u_int8_t*>(val));
}

pq_async::oid pgval_to_oid(char* val, int len, int fmt)
{
    if(len == -1)
        return 0;

    if(!fmt)
        return pq_async::str_to_num<uint32_t>(std::string(val, len));

    uint32_t value = 0; //*((int32_t*)val);
    pq_async::swap4((int32_t*)val, (int32_t*)&value, false);

    return value;
}

pq_async::cidr pgval_to_cidr(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::cidr();

    if(!fmt)
        return pq_async::cidr(std::string(val, len).c_str());
    
    pq_async::cidr addr;
    uint8_t f = *val;
    if(f != PGSQL_AF_INET && f != PGSQL_AF_INET6)
        throw pq_async::exception("Unsupported family type!");
    addr._family = f == PGSQL_AF_INET ?
        net_family::pq_net_family_inet :
        net_family::pq_net_family_inet6;
    
    addr._bits = *(val +1);
    size_t nb = *(val +3);
    //size_t nb = 4 + (addr._family == net_family::pq_net_family_inet ? 4 : 16);
    for(size_t i = 0; i < nb; ++i)
        addr._ipaddr[i] = *(val +(i +4));
    
    return addr;
}

pq_async::inet pgval_to_inet(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::inet();

    if(!fmt)
        return pq_async::inet(std::string(val, len).c_str());

    pq_async::inet addr;
    uint8_t f = *val;
    if(f != PGSQL_AF_INET && f != PGSQL_AF_INET6)
        throw pq_async::exception("Unsupported family type!");
    addr._family = f == PGSQL_AF_INET ?
        net_family::pq_net_family_inet :
        net_family::pq_net_family_inet6;
    
    addr._bits = *(val +1);
    
    size_t nb = *(val +3);
    //size_t nb = 4 + (addr._family == net_family::pq_net_family_inet ? 4 : 16);
    for(size_t i = 0; i < nb; ++i)
        addr._ipaddr[i] = *(val +(i +4));
    
    return addr;
}

pq_async::macaddr pgval_to_macaddr(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::macaddr();

    if(!fmt)
        return pq_async::macaddr(std::string(val, len).c_str());
    
    pq_async::macaddr m;
    m.a = *(val +0);
    m.b = *(val +1);
    m.c = *(val +2);
    m.d = *(val +3);
    m.e = *(val +4);
    m.f = *(val +5);
    
    return m;
}

pq_async::macaddr8 pgval_to_macaddr8(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::macaddr8();

    if(!fmt)
        return pq_async::macaddr8(std::string(val, len).c_str());

    pq_async::macaddr8 m;
    m.a = *(val +0);
    m.b = *(val +1);
    m.c = *(val +2);
    m.d = *(val +3);
    m.e = *(val +4);
    m.f = *(val +5);
    m.g = *(val +6);
    m.h = *(val +7);
    
    return m;
}

pq_async::point pgval_to_point(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::point();
    
    if(!fmt)
        return pq_async::point(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    pq_async::point p;
    
    p._x = pgval_to_double(val, dl, fmt);
    p._y = pgval_to_double(val +dl, dl, fmt);
    
    return p;
}
pq_async::line pgval_to_line(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::line();
    
    if(!fmt)
        return pq_async::line(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    pq_async::line l;
    
    l._a = pgval_to_double(val, dl, fmt);
    l._b = pgval_to_double(val +dl, dl, fmt);
    l._c = pgval_to_double(val +(dl*2), dl, fmt);
    return l;
}
pq_async::lseg pgval_to_lseg(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::lseg();
    
    if(!fmt)
        return pq_async::lseg(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    int ps = dl *2;
    pq_async::lseg l;
    
    l._p[0] = pgval_to_point(val, ps, fmt);
    l._p[1] = pgval_to_point(val +ps, ps, fmt);
    
    return l;
}
pq_async::box pgval_to_box(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::box();
    
    if(!fmt)
        return pq_async::box(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    int ps = dl *2;
    pq_async::box b;
    
    b._high = pgval_to_point(val, ps, fmt);
    b._low = pgval_to_point(val +ps, ps, fmt);
    
    return b;
}
pq_async::path pgval_to_path(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::path();
    
    if(!fmt)
        return pq_async::path(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    int ps = dl *2;
    int o = sizeof(char) + sizeof(int32_t);
    pq_async::path p;
    
    p._closed = val[0] == (char)1;
    
    int32_t nb = pgval_to_int32(val +(sizeof(char)), sizeof(int32_t), fmt);
    for(int32_t i = 0; i < nb; ++i)
        p._p.push_back(pgval_to_point(val + (i*ps) + o, ps, fmt));
    
    return p;
}
pq_async::polygon pgval_to_polygon(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::polygon();
    
    if(!fmt)
        return pq_async::polygon(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    int ps = dl *2;
    int o = sizeof(int32_t);
    pq_async::polygon p;
    
    int32_t nb = pgval_to_int32(val, sizeof(int32_t), fmt);
    for(int32_t i = 0; i < nb; ++i)
        p._p.push_back(pgval_to_point(val + (i*ps) + o, ps, fmt));
    
    return p;
}
pq_async::circle pgval_to_circle(char* val, int len, int fmt)
{
    if(len == -1)
        return pq_async::circle();
    
    if(!fmt)
        return pq_async::circle(std::string(val, len).c_str());
    
    int dl = sizeof(double);
    int ps = dl *2;
    pq_async::circle c;
    
    c._center = pgval_to_point(val, ps, fmt);
    c._radius = pgval_to_double(val +ps, dl, fmt);
    
    return c;
}


template <>
pq_async::int4range pgval_to_range<pq_async::int4range>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    int4range r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_int32(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_int32(out, l, fmt);
        out += l;
    }
    
    return r;
}

template <>
pq_async::int8range pgval_to_range<pq_async::int8range>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    int8range r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_int64(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_int64(out, l, fmt);
        out += l;
    }
    
    return r;
}

template <>
pq_async::numrange pgval_to_range<pq_async::numrange>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    numrange r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_numeric(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_numeric(out, l, fmt);
        out += l;
    }
    
    return r;
}

template <>
pq_async::tsrange pgval_to_range<pq_async::tsrange>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    tsrange r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_timestamp(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_timestamp(out, l, fmt);
        out += l;
    }
    
    return r;
}

template <>
pq_async::tstzrange pgval_to_range<pq_async::tstzrange>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    tstzrange r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_timestamp_tz(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_timestamp_tz(out, l, fmt);
        out += l;
    }
    
    return r;
}

template <>
pq_async::daterange pgval_to_range<pq_async::daterange>(
    char* val, int len, int fmt
)
{
    char* out = val;
    
    daterange r;
    r._flags = (range_flag)(*out++);
    if(r.has_lbound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._lb = pgval_to_date(out, l, fmt);
        out += l;
    }
    if(r.has_ubound()){
        int32_t l = pgval_to_int32(out, sizeof(int32_t), fmt);
        out += sizeof(int32_t);
        r._ub = pgval_to_date(out, l, fmt);
        out += l;
    }
    
    return r;
}


/////////////////////////////////////
// new_parameter implementations...//
/////////////////////////////////////

pq_async::parameter* new_null_parameter()
{
    pq_async::parameter* params =
        //new pq_async::parameter(TEXTOID, NULL, 0, 0);
        // test with zero OID
        new pq_async::parameter(0, NULL, 0, 0);

    return params;
}

pq_async::parameter* new_parameter(const std::string& value)
{
    char* values = new char[value.size() +1];
    std::copy(value.begin(), value.end(), values);
    values[value.size()] = '\0';

    pq_async::parameter* params =
        new pq_async::parameter(TEXTOID, values, value.size() + 1, 0);

    return params;
}

pq_async::parameter* new_parameter(const char* value)
{
    return new_parameter(std::string(value));
}

pq_async::parameter* new_parameter(bool value)
{
    char* val = new char[1];
    val[0] = value ? 1 : 0;
    return new pq_async::parameter(BOOLOID, val, sizeof(char), 1);
}

pq_async::parameter* new_parameter(int16_t value)
{
    char* val = new char[sizeof(int16_t)];
    pq_async::swap2(&value, (int16_t*)val, true);
    return new pq_async::parameter(INT2OID, val, sizeof(int16_t), 1);
}

pq_async::parameter* new_parameter(int32_t value)
{
    char* val = new char[sizeof(int32_t)];
    pq_async::swap4(&value, (int32_t*)val, true);
    return new pq_async::parameter(INT4OID, val, sizeof(int32_t), 1);
}

pq_async::parameter* new_parameter(int64_t value)
{
    char* val = new char[sizeof(int64_t)];
    pq_async::swap8(&value, (int64_t*)val, true);

    return new pq_async::parameter(INT8OID, val, sizeof(int64_t), 1);
}

pq_async::parameter* new_parameter(float value)
{
    char* val = new char[sizeof(float)];
    pq_async::swap4((int32_t*)&value, (int32_t*)val, true);

    return new pq_async::parameter(FLOAT4OID, val, sizeof(float), 1);
}

pq_async::parameter* new_parameter(double value)
{
    char* val = new char[sizeof(double)];
    pq_async::swap8((int64_t*)&value, (int64_t*)val, true);

    return new pq_async::parameter(FLOAT8OID, val, sizeof(double), 1);
}


pq_async::parameter* new_parameter(const pq_async::time& value)
{
    int64_t val = value.pgticks();// - POSTGRES_EPOCH_USEC;
    char* out_val = new char[sizeof(int64_t)];
    pq_async::swap8(&val, (int64_t*)out_val, true);
    return new pq_async::parameter(TIMEOID, out_val, sizeof(int64_t), 1);
}

pq_async::parameter* new_parameter(const pq_async::time_tz& value)
{
    int64_t val = value.pgticks() - POSTGRES_EPOCH_USEC;
    int32_t len = sizeof(int64_t) + sizeof(int32_t);
    char* out_val = new char[len];
    pq_async::swap8(&val, (int64_t*)out_val, true);
    
    char* zout = out_val + sizeof(int64_t);
    // sending time in UTC
    zout[0] = 0;
    zout[1] = 0;
    zout[2] = 0;
    zout[3] = 0;
    
    return new pq_async::parameter(TIMETZOID, out_val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::timestamp& value)
{
    int64_t val = value.pgticks() - POSTGRES_EPOCH_USEC;
    char* out_val = new char[sizeof(int64_t)];
    pq_async::swap8(&val, (int64_t*)out_val, true);
    return new pq_async::parameter(TIMESTAMPOID, out_val, sizeof(int64_t), 1);
}

pq_async::parameter* new_parameter(const pq_async::timestamp_tz& value)
{
    int64_t val = value.pgticks() - POSTGRES_EPOCH_USEC;
    char* out_val = new char[sizeof(int64_t)];
    pq_async::swap8(&val, (int64_t*)out_val, true);
    return new pq_async::parameter(TIMESTAMPTZOID, out_val, sizeof(int64_t), 1);
}

pq_async::parameter* new_parameter(const pq_async::date& value)
{
    int32_t val = value.pgticks() - POSTGRES_EPOCH_JDATE;
    char* out_val = new char[sizeof(int32_t)];
    pq_async::swap4(&val, (int32_t*)out_val, true);
    return new pq_async::parameter(DATEOID, out_val, sizeof(int32_t), 1);
}

pq_async::parameter* new_parameter(const pq_async::interval& value)
{
    int64_t tval = value.time_of_day().to_duration().count();
    int32_t dval = value.days();
    int32_t mval = value.months();
    int32_t len = sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t);
    char* out_val = new char[len];
    pq_async::swap8(&tval, (int64_t*)out_val, true);
    pq_async::swap4(
        &dval,
        (int32_t*)(out_val + sizeof(int64_t)),
        true
    );
    pq_async::swap4(
        &mval,
        (int32_t*)(out_val + (sizeof(int64_t) + sizeof(int32_t))),
        true
    );

    return new pq_async::parameter(INTERVALOID, out_val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::numeric& value)
{
    int numlen = (int)sizeof(int16_t) * (4 + value.ndigits);
    char* val = new char[numlen];
    
    int16_t* out = (int16_t*)val;
    *out++ = htons((int16_t)value.ndigits);
    *out++ = htons((int16_t)value.weight);
    *out++ = htons((int16_t)value.sign);
    *out++ = htons((int16_t)value.dscale);
    
    if(value.digits)
        for(int i = 0; i < value.ndigits; i++)
            *out++ = htons((int16_t)value.digits[i]);
    
    return new pq_async::parameter(NUMERICOID, val, numlen, 1);
}

pq_async::parameter* new_parameter(const pq_async::money& value)
{
    char* val = new char[sizeof(int64_t)];
    pq_async::swap8(&value._val, (int64_t*)val, true);
    
    return new pq_async::parameter(CASHOID, val, sizeof(int64_t), 1);
}


pq_async::parameter* new_parameter(const pq_async::json& value)
{
    std::string sval = value.dump();
    char* values = new char[sval.size() +2];

    values[0] = 1;
    std::copy(sval.begin(), sval.end(), values +1);
    values[sval.size() +1] = '\0';

    pq_async::parameter* params =
        new pq_async::parameter(JSONBOID, values, value.size() + 2, 0);

    return params;
}

pq_async::parameter* new_parameter(const std::vector<int8_t>& value)
{
    std::string str = "\\x" + hex_to_str((u_char*)value.data(), value.size());

    char* values = new char[str.size() +1];
    std::copy(str.begin(), str.end(), values);
    values[str.size()] = '\0';

    pq_async::parameter* params =
        new pq_async::parameter(BYTEAOID, values, str.size() + 1, 0);

    return params;
}

pq_async::parameter* new_parameter(const pq_async::uuid& value)
{
    std::string sval = value;
    char* values = new char[sval.size() +1];
    std::copy(sval.begin(), sval.end(), values);
    values[sval.size()] = '\0';

    pq_async::parameter* params =
        new pq_async::parameter(UUIDOID, values, sval.size() + 1, 0);
    return params;
}

pq_async::parameter* new_parameter(const pq_async::oid& value)
{
    char* val = new char[sizeof(uint32_t)];
    uint32_t value_val = value;
    pq_async::swap4((int32_t*)&value_val, (int32_t*)val, true);
    return new pq_async::parameter(OIDOID, val, sizeof(uint32_t), 1);
}

pq_async::parameter* new_parameter(const pq_async::cidr& value)
{
    size_t nb = (value._family == net_family::pq_net_family_inet ? 4 : 16);
    char* val = new char[nb +4];
    char* out = val;
    *out++ = value._family == net_family::pq_net_family_inet ?
        PGSQL_AF_INET : PGSQL_AF_INET6;
    *out++ = value._bits;
    *out++ = 1;
    *out++ = nb;
    for(size_t i = 0; i < nb; ++i)
        *out++ = value._ipaddr[i];
    
    return new pq_async::parameter(CIDROID, val, nb +4, 1);
}

pq_async::parameter* new_parameter(const pq_async::inet& value)
{
    size_t nb = (value._family == net_family::pq_net_family_inet ? 4 : 16);
    char* val = new char[nb +4];
    char* out = val;
    *out++ = value._family == net_family::pq_net_family_inet ?
        PGSQL_AF_INET : PGSQL_AF_INET6;
    *out++ = value._bits;
    *out++ = 0;
    *out++ = nb;
    for(size_t i = 0; i < nb; ++i)
        *out++ = value._ipaddr[i];
    
    return new pq_async::parameter(INETOID, val, nb +4, 1);
}

pq_async::parameter* new_parameter(const pq_async::macaddr& value)
{
    char* val = new char[6];
    char* out = val;
    
    *out++ = value.a;
    *out++ = value.b;
    *out++ = value.c;
    *out++ = value.d;
    *out++ = value.e;
    *out++ = value.f;
    
    return new pq_async::parameter(MACADDROID, val, 6, 1);
}

pq_async::parameter* new_parameter(const pq_async::macaddr8& value)
{
    char* val = new char[8];
    char* out = val;
    
    *out++ = value.a;
    *out++ = value.b;
    *out++ = value.c;
    *out++ = value.d;
    *out++ = value.e;
    *out++ = value.f;
    *out++ = value.g;
    *out++ = value.h;
    
    return new pq_async::parameter(MACADDR8OID, val, 8, 1);
}

pq_async::parameter* new_parameter(const pq_async::point& value)
{
    int dl = sizeof(double);
    int len = sizeof(double) *2;
    char* val = new char[len];
    
    pq_async::swap8((int64_t*)&value._x, (int64_t*)val, true);
    pq_async::swap8((int64_t*)&value._y, (int64_t*)(val +(dl*1)), true);
    
    return new pq_async::parameter(POINTOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::line& value)
{
    int dl = sizeof(double);
    int len = sizeof(double) *3;
    char* val = new char[len];
    
    pq_async::swap8((int64_t*)&value._a, (int64_t*)val, true);
    pq_async::swap8((int64_t*)&value._b, (int64_t*)(val +(dl*1)), true);
    pq_async::swap8((int64_t*)&value._c, (int64_t*)(val +(dl*2)), true);
    
    return new pq_async::parameter(LINEOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::lseg& value)
{
    int dl = sizeof(double);
    int len = sizeof(double) *4;
    char* val = new char[len];
    
    pq_async::swap8((int64_t*)&value._p[0]._x, (int64_t*)val, true);
    pq_async::swap8((int64_t*)&value._p[0]._y, (int64_t*)(val +(dl*1)), true);
    pq_async::swap8((int64_t*)&value._p[1]._x, (int64_t*)(val +(dl*2)), true);
    pq_async::swap8((int64_t*)&value._p[1]._y, (int64_t*)(val +(dl*3)), true);
    
    return new pq_async::parameter(LSEGOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::box& value)
{
    int dl = sizeof(double);
    int len = sizeof(double) *4;
    char* val = new char[len];
    
    pq_async::swap8((int64_t*)&value._high._x, (int64_t*)val, true);
    pq_async::swap8((int64_t*)&value._high._y, (int64_t*)(val +(dl*1)), true);
    pq_async::swap8((int64_t*)&value._low._x, (int64_t*)(val +(dl*2)), true);
    pq_async::swap8((int64_t*)&value._low._y, (int64_t*)(val +(dl*3)), true);
    
    return new pq_async::parameter(BOXOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::path& value)
{
    int dl = sizeof(double);
    // point size
    int ps = dl *2;
    int nb = (int32_t)value._p.size();
    // offset
    int o = sizeof(char) + sizeof(int32_t);
    int len = o + (value._p.size() * ps);
    char* val = new char[len];

    *val = value._closed ? (char)1 : (char)0;
    pq_async::swap4((int32_t*)&nb, (int32_t*)(val +1), true);
    
    for(int32_t i = 0; i < nb; ++i){
        pq_async::swap8(
            (int64_t*)&value._p[i]._x, (int64_t*)(val + o + (ps*i)), true
        );
        pq_async::swap8(
            (int64_t*)&value._p[i]._y, (int64_t*)(val + o + (ps*i) + dl), true
        );
    }	
    return new pq_async::parameter(PATHOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::polygon& value)
{
    int dl = sizeof(double);
    // point size
    int ps = dl *2;
    int nb = (int32_t)value._p.size();
    // offset
    int o = sizeof(int32_t);
    int len = o + (value._p.size() * ps);
    char* val = new char[len];
    
    pq_async::swap4((int32_t*)&nb, (int32_t*)(val), true);
    
    for(int32_t i = 0; i < nb; ++i){
        pq_async::swap8(
            (int64_t*)&value._p[i]._x, (int64_t*)(val + o + (ps*i)), true
        );
        pq_async::swap8(
            (int64_t*)&value._p[i]._y, (int64_t*)(val + o + (ps*i) + dl), true
        );
    }
    return new pq_async::parameter(POLYGONOID, val, len, 1);
}
pq_async::parameter* new_parameter(const pq_async::circle& value)
{
    int dl = sizeof(double);
    int len = sizeof(double) *3;
    char* val = new char[len];
    
    pq_async::swap8((int64_t*)&value._center._x, (int64_t*)val, true);
    pq_async::swap8((int64_t*)&value._center._y, (int64_t*)(val +(dl*1)), true);
    pq_async::swap8((int64_t*)&value._radius, (int64_t*)(val +(dl*2)), true);
    
    return new pq_async::parameter(CIRCLEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::int4range& value)
{
    int il = sizeof(int32_t);
    int vl = sizeof(int32_t);
    int len = 1;
    if(value.has_lbound())
        len += (il + vl);
    if(value.has_ubound())
        len += (il + vl);
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        pq_async::swap4((int32_t*)&value._lb, (int32_t*)out, true);
        out += vl;
    }
    if(value.has_ubound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        pq_async::swap4((int32_t*)&value._ub, (int32_t*)out, true);
        out += vl;
    }
    
    return new pq_async::parameter(INT4RANGEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::int8range& value)
{
    int il = sizeof(int32_t);
    int vl = sizeof(int64_t);
    int len = 1;
    if(value.has_lbound())
        len += (il + vl);
    if(value.has_ubound())
        len += (il + vl);
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        pq_async::swap8((int64_t*)&value._lb, (int64_t*)out, true);
        out += vl;
    }
    if(value.has_ubound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        pq_async::swap8((int64_t*)&value._ub, (int64_t*)out, true);
        out += vl;
    }
    
    return new pq_async::parameter(INT8RANGEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::numrange& value)
{
    int il = sizeof(int32_t);
    int len = 1;
    if(value.has_lbound()){
        int numlen = (int)sizeof(int16_t) * (4 + value._lb.ndigits);
        len += (il + numlen);
    }
    if(value.has_ubound()){
        int numlen = (int)sizeof(int16_t) * (4 + value._ub.ndigits);
        len += (il + numlen);
    }
    
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        int numlen = (int)sizeof(int16_t) * (4 + value._lb.ndigits);
        pq_async::swap4((int32_t*)&numlen, (int32_t*)out, true);
        out += il;
        
        int16_t* nout = (int16_t*)out;
        *nout++ = htons((int16_t)value._lb.ndigits);
        *nout++ = htons((int16_t)value._lb.weight);
        *nout++ = htons((int16_t)value._lb.sign);
        *nout++ = htons((int16_t)value._lb.dscale);
        
        if(value._lb.digits)
            for(int i = 0; i < value._lb.ndigits; i++)
                *nout++ = htons((int16_t)value._lb.digits[i]);
        
        out += numlen;
    }
    if(value.has_ubound()){
        int numlen = (int)sizeof(int16_t) * (4 + value._ub.ndigits);
        pq_async::swap4((int32_t*)&numlen, (int32_t*)out, true);
        out += il;
        
        int16_t* nout = (int16_t*)out;
        *nout++ = htons((int16_t)value._ub.ndigits);
        *nout++ = htons((int16_t)value._ub.weight);
        *nout++ = htons((int16_t)value._ub.sign);
        *nout++ = htons((int16_t)value._ub.dscale);
        
        if(value._ub.digits)
            for(int i = 0; i < value._ub.ndigits; i++)
                *nout++ = htons((int16_t)value._ub.digits[i]);
        
        out += numlen;
    }
    
    return new pq_async::parameter(NUMRANGEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::tsrange& value)
{
    int il = sizeof(int32_t);
    int vl = sizeof(int64_t);
    int len = 1;
    if(value.has_lbound())
        len += (il + vl);
    if(value.has_ubound())
        len += (il + vl);
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int64_t val = value._lb.pgticks() - POSTGRES_EPOCH_USEC;
        pq_async::swap8(&val, (int64_t*)out, true);
        out += vl;
    }
    if(value.has_ubound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int64_t val = value._ub.pgticks() - POSTGRES_EPOCH_USEC;
        pq_async::swap8(&val, (int64_t*)out, true);
        out += vl;
    }
    
    return new pq_async::parameter(TSRANGEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::tstzrange& value)
{
    int il = sizeof(int32_t);
    int vl = sizeof(int64_t);
    int len = 1;
    if(value.has_lbound())
        len += (il + vl);
    if(value.has_ubound())
        len += (il + vl);
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int64_t val = value._lb.pgticks() - POSTGRES_EPOCH_USEC;
        pq_async::swap8(&val, (int64_t*)out, true);
        out += vl;
    }
    if(value.has_ubound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int64_t val = value._ub.pgticks() - POSTGRES_EPOCH_USEC;
        pq_async::swap8(&val, (int64_t*)out, true);
        out += vl;
    }
    
    return new pq_async::parameter(TSTZRANGEOID, val, len, 1);
}

pq_async::parameter* new_parameter(const pq_async::daterange& value)
{
    int il = sizeof(int32_t);
    int vl = sizeof(int32_t);
    int len = 1;
    if(value.has_lbound())
        len += (il + vl);
    if(value.has_ubound())
        len += (il + vl);
    char* val = new char[len];
    char* out = val;
    
    *out++ = (char)value._flags;
    if(value.has_lbound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int32_t val = value._lb.pgticks() - POSTGRES_EPOCH_JDATE;
        pq_async::swap4(&val, (int32_t*)out, true);
        out += vl;
    }
    if(value.has_ubound()){
        pq_async::swap4((int32_t*)&vl, (int32_t*)out, true);
        out += il;
        int32_t val = value._ub.pgticks() - POSTGRES_EPOCH_JDATE;
        pq_async::swap4(&val, (int32_t*)out, true);
        out += vl;
    }
    
    return new pq_async::parameter(DATERANGEOID, val, len, 1);
}



void vec_append(std::vector<char> &vec, char* buf, int len)
{
    std::vector<char>::iterator it = vec.begin();
    it += vec.size();
    vec.insert(it, buf, buf + len);
}


void get_array_oid_and_dim(char* val, int len, int fmt, int& oid, int& dim)
{
    if(!fmt)
        throw pq_async::exception("Text format is not supported!");

    // dimension count.
    int32_t svrDimCount = 0;
    pq_async::swap4((int32_t*)val, &svrDimCount, false);
    val += 4;

    dim = svrDimCount;

    // skip NULL flag.
    val += 4;

    // verify element OID.
    int32_t eleOid = 0;
    pq_async::swap4((int32_t*)val, &eleOid, false);
    val += 4;

    oid = eleOid;
}

//////////////////////////
// array specialization //
//////////////////////////

#define PQ_ASYNC_NEW_PARAMETER(__type, __name, __dim, __eleOid, __arrEleOid) \
pq_async::parameter* new_parameter( \
    const boost::multi_array<__type, __dim> & value) \
{ \
    return new_parameter_gen<__type, __dim>(value, __eleOid, __arrEleOid); \
} \
template<> \
boost::multi_array< __type, __dim > \
val_from_pgparam< boost::multi_array< __type, __dim > >( \
int oid, char* val, int len, int format) \
{ \
    return pgval_to_array< __type, __dim >( \
        val, len, format \
    ); \
}


#define PQ_ASYNC_NEW_PARAMETER_DIM(__dim) \
PQ_ASYNC_NEW_PARAMETER(std::string, string, __dim, TEXTOID, TEXTARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(char*, char, __dim, TEXTOID, TEXTARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(int16_t, int16, __dim, INT2OID, INT2ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(int32_t, int32, __dim, INT4OID, INT4ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(int64_t, int64, __dim, INT8OID, INT8ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(float, float, __dim, FLOAT4OID, FLOAT4ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(double, double, __dim, FLOAT8OID, FLOAT8ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::numeric, numeric, __dim, NUMERICOID, NUMERICARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::money, money, __dim, CASHOID, MONEYARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::time, time, __dim, TIMEOID, TIMEARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::time_tz, time_tz, __dim, TIMETZOID, TIMETZARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::timestamp, timestamp, __dim, TIMESTAMPOID, TIMESTAMPARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::timestamp_tz, timestamp_tz, __dim, TIMESTAMPTZOID, TIMESTAMPTZARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::date, date, __dim, DATEOID, DATEARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::interval, interval, __dim, INTERVALOID, INTERVALARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::json, json, __dim, JSONBOID, JSONBARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(std::vector<int8_t>, bytea, __dim, BYTEAOID, BYTEAARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::uuid, uuid, __dim, UUIDOID, UUIDARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::oid, oid, __dim, OIDOID, OIDARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::cidr, cidr, __dim, CIDROID, CIDRARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::inet, inet, __dim, INETOID, INETARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::macaddr, macaddr, __dim, MACADDROID, MACADDRARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::macaddr8, macaddr8, __dim, MACADDR8OID, MACADDR8ARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::point, point, __dim, POINTOID, POINTARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::line, line, __dim, LINEOID, LINEARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::lseg, lseg, __dim, LSEGOID, LSEGARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::box, box, __dim, BOXOID, BOXARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::path, path, __dim, PATHOID, PATHARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::polygon, polygon, __dim, POLYGONOID, POLYGONARRAYOID); \
PQ_ASYNC_NEW_PARAMETER(pq_async::circle, circle, __dim, CIRCLEOID, CIRCLEARRAYOID); \

PQ_ASYNC_NEW_PARAMETER_DIM(1);
PQ_ASYNC_NEW_PARAMETER_DIM(2);
PQ_ASYNC_NEW_PARAMETER_DIM(3);
PQ_ASYNC_NEW_PARAMETER_DIM(4);
PQ_ASYNC_NEW_PARAMETER_DIM(5);


// val_from_pgparam...

template<>
bool val_from_pgparam<bool>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<int32_t>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format);
        case TEXTOID:{
            std::string str_val = pgval_to_string(val, len, format);
            if(str_val == "false" || str_val == "0")
                return false;
            else
                return true;
        }
        case INT2OID:
            return (bool)pgval_to_int16(val, len, format);
        case INT4OID:
            return pgval_to_int32(val, len, format);
        case INT8OID:
            return (bool)pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return (bool)pgval_to_float(val, len, format);
        case FLOAT8OID:
            return (bool)pgval_to_double(val, len, format);
        case NUMERICOID:
            return
                val_from_pgparam<std::string>(oid, val, len, format) == "0" ?
                false : true;
        //case TIMESTAMPOID:
        //	return pgval_to_timestamp(val, len, format);
        //case DATEOID:
        //	return pgval_to_date(val, len, format);
        //case TIMEOID:
        //	return pgval_to_time(val, len, format);
        //case INTERVALOID:
        //	return pgval_to_interval(val, len, format);
        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
std::string val_from_pgparam<std::string>(
    int oid, char* val, int len, int format)
{
    // std::cout << "len: " << len << std::endl;
    // for(int i = 0; i < len; ++i)
    // 	std::cout << std::dec << (int)val[i] << ", ";
    // std::cout << std::endl;
    
    if(!format)// if result in text format.
        return std::string(val, len);
    
    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? "true" : "false";
        case JSONBOID:{
            std::string jsonText = pgval_to_string(val, len, format);
            if(jsonText.c_str()[0] == 1)
                jsonText = jsonText.substr(1, jsonText.size() -1);
            return jsonText;
        }
        case JSONOID:
        case VARCHAROID:
        case BPCHAROID:
        case TEXTOID:
            return pgval_to_string(val, len, format);
        case INT2OID:
            return num_to_str(pgval_to_int16(val, len, format));
        case INT4OID:
            return num_to_str(pgval_to_int32(val, len, format));
        case INT8OID:
            return num_to_str(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return num_to_str(pgval_to_float(val, len, format));
        case FLOAT8OID:
            return num_to_str(pgval_to_double(val, len, format));
        case NUMERICOID:
            return pgval_to_numeric(val, len, format);
        case TIMEOID:
            return pgval_to_time(val, len, format).iso_string();
        case TIMETZOID:
            return pgval_to_time_tz(val, len, format).iso_string();
        case TIMESTAMPOID:
            return pgval_to_timestamp(val, len, format).iso_string();
        case TIMESTAMPTZOID:
            return pgval_to_timestamp_tz(val, len, format).iso_string();
        case DATEOID:
            return pgval_to_date(val, len, format).iso_string();
        case INTERVALOID:
            return pgval_to_interval(val, len, format).iso_string();
        
        case CIDROID:
            return pgval_to_cidr(val, len, format);
        case INETOID:
            return pgval_to_inet(val, len, format);
        
        case MACADDROID:
            return pgval_to_macaddr(val, len, format);
        case MACADDR8OID:
            return pgval_to_macaddr8(val, len, format);

        case POINTOID:
            return pgval_to_point(val, len, format);
        case LINEOID:
            return pgval_to_line(val, len, format);
        case LSEGOID:
            return pgval_to_lseg(val, len, format);
        case BOXOID:
            return pgval_to_box(val, len, format);
        case PATHOID:
            return pgval_to_path(val, len, format);
        case POLYGONOID:
            return pgval_to_polygon(val, len, format);
        case CIRCLEOID:
            return pgval_to_circle(val, len, format);
        
        case INT4RANGEOID:
            return pgval_to_range<pq_async::int4range>(val, len, format);
        case INT8RANGEOID:
            return pgval_to_range<pq_async::int8range>(val, len, format);
        case NUMRANGEOID:
            return pgval_to_range<pq_async::numrange>(val, len, format);
        case TSRANGEOID:
            return pgval_to_range<pq_async::tsrange>(val, len, format);
        case TSTZRANGEOID:
            return pgval_to_range<pq_async::tstzrange>(val, len, format);
        case DATERANGEOID:
            return pgval_to_range<pq_async::daterange>(val, len, format);
        
        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::json val_from_pgparam<pq_async::json>(
    int oid, char* val, int len, int format)
{
    if(!format)// if result in text format.
        return pq_async::json::parse(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:{
            pq_async::json j = pgval_to_bool(val, len, format);
            return j;
        }
        case JSONBOID:{
            std::string jsonText = pgval_to_string(val, len, format);
            if(jsonText.c_str()[0] == 1)
                jsonText = jsonText.substr(1, jsonText.size() -1);
            return pq_async::json::parse(jsonText);
        }
        case JSONOID:
            return pq_async::json::parse(pgval_to_string(val, len, format));
        case TEXTOID:{
            pq_async::json j = pgval_to_string(val, len, format);
            return j;
        }
        case INT2OID:{
            pq_async::json j = pgval_to_int16(val, len, format);
            return j;
        }
        case INT4OID:{
            pq_async::json j = pgval_to_int32(val, len, format);
            return j;
        }
        case INT8OID:{
            pq_async::json j = pgval_to_int64(val, len, format);
            return j;
        }
        case FLOAT4OID:{
            pq_async::json j = pgval_to_float(val, len, format);
            return j;
        }
        case FLOAT8OID:{
            pq_async::json j = pgval_to_double(val, len, format);
            return j;
        }
        case NUMERICOID:{
            numeric num = pgval_to_numeric(val, len, format);
            pq_async::json j = (std::string)num;
            return j;
        }
        case CASHOID:{
            money cash = pgval_to_money(val, len, format);
            pq_async::json j = (std::string)cash;
            return j;
        }
        
        case TIMEOID:
            return pgval_to_time(val, len, format).iso_string();
        case TIMETZOID:
            return pgval_to_time_tz(val, len, format).iso_string();
        case TIMESTAMPOID:
            return pgval_to_timestamp(val, len, format).iso_string();
        case TIMESTAMPTZOID:
            return pgval_to_timestamp_tz(val, len, format).iso_string();
        case DATEOID:
            return pgval_to_date(val, len, format).iso_string();
        case INTERVALOID:
            return pgval_to_interval(val, len, format).iso_string();

        case CIDROID:
            return (std::string)pgval_to_cidr(val, len, format);
        case INETOID:
            return (std::string)pgval_to_inet(val, len, format);
        case MACADDROID:
            return (std::string)pgval_to_macaddr(val, len, format);
        case MACADDR8OID:
            return (std::string)pgval_to_macaddr8(val, len, format);
        
        case POINTOID:
            return (std::string)pgval_to_point(val, len, format);
        case LINEOID:
            return (std::string)pgval_to_line(val, len, format);
        case LSEGOID:
            return (std::string)pgval_to_lseg(val, len, format);
        case BOXOID:
            return (std::string)pgval_to_box(val, len, format);
        case PATHOID:
            return (std::string)pgval_to_path(val, len, format);
        case POLYGONOID:
            return (std::string)pgval_to_polygon(val, len, format);
        case CIRCLEOID:
            return (std::string)pgval_to_circle(val, len, format);

        case INT4RANGEOID:
            return (std::string)pgval_to_range<pq_async::int4range>(
                val, len, format
            );
        case INT8RANGEOID:
            return (std::string)pgval_to_range<pq_async::int8range>(
                val, len, format
            );
        case NUMRANGEOID:
            return (std::string)pgval_to_range<pq_async::numrange>(
                val, len, format
            );
        case TSRANGEOID:
            return (std::string)pgval_to_range<pq_async::tsrange>(
                val, len, format
            );
        case TSTZRANGEOID:
            return (std::string)pgval_to_range<pq_async::tstzrange>(
                val, len, format
            );
        case DATERANGEOID:
            return (std::string)pgval_to_range<pq_async::daterange>(
                val, len, format
            );

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}



template<>
pq_async::numeric val_from_pgparam<pq_async::numeric>(
    int oid, char* val, int len, int format)
{
    if(!format)// if result in text format.
        return pgval_to_numeric(val, len, format);

    // else result is in bin format.
    switch(oid){
        case NUMERICOID:
            return pgval_to_numeric(val, len, format);
        case CASHOID:
            return pgval_to_money(val, len, format);
        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::money val_from_pgparam<pq_async::money>(
    int oid, char* val, int len, int format)
{
    if(!format)// if result in text format.
        return pgval_to_money(val, len, format);
    
    // else result is in bin format.
    switch(oid){
        case CASHOID:
            return pgval_to_money(val, len, format);
        case NUMERICOID:
            return pq_async::money::from_numeric(
                pgval_to_numeric(val, len, format)
            );
        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
int16_t val_from_pgparam<int16_t>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<int16_t>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? 1 : 0;
        case TEXTOID:
            return str_to_num<int16_t>(pgval_to_string(val, len, format));
        case INT2OID:
            return pgval_to_int16(val, len, format);
        case INT4OID:
            return (int16_t)pgval_to_int32(val, len, format);
        case INT8OID:
            return (int16_t)pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return (int16_t)pgval_to_float(val, len, format);
        case FLOAT8OID:
            return (int16_t)pgval_to_double(val, len, format);
        case NUMERICOID:
            return str_to_num<int16_t>(
                val_from_pgparam<std::string>(oid, val, len, format)
            );

        //case TIMESTAMPOID:
        //	return pgval_to_timestamp(val, len, format);
        //case DATEOID:
        //	return pgval_to_date(val, len, format);
        //case TIMEOID:
        //	return pgval_to_time(val, len, format);
        //case INTERVALOID:
        //	return pgval_to_interval(val, len, format);
        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
int32_t val_from_pgparam<int32_t>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<int32_t>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? 1 : 0;
        case TEXTOID:
            return str_to_num<int32_t>(pgval_to_string(val, len, format));
        case INT2OID:
            return (int32_t)pgval_to_int16(val, len, format);
        case INT4OID:
            return pgval_to_int32(val, len, format);
        case INT8OID:
            return (int32_t)pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return (int32_t)pgval_to_float(val, len, format);
        case FLOAT8OID:
            return (int32_t)pgval_to_double(val, len, format);
        case NUMERICOID:
            return str_to_num<int32_t>(
                val_from_pgparam<std::string>(oid, val, len, format)
            );

        case TIMEOID:
            return (int32_t)pgval_to_time(val, len, format).pgticks();
        case TIMETZOID:
            return (int32_t)pgval_to_time_tz(val, len, format).pgticks();
        case TIMESTAMPOID:
            return (int32_t)pgval_to_timestamp(val, len, format).pgticks();
        case TIMESTAMPTZOID:
            return (int32_t)pgval_to_timestamp_tz(val, len, format).pgticks();
        case DATEOID:
            return (int32_t)pgval_to_date(val, len, format).pgticks();
        case INTERVALOID:
            return (int32_t)pgval_to_interval(val, len, format).pgticks();

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
int64_t val_from_pgparam<int64_t>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<int64_t>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? 1 : 0;
        case TEXTOID:
            return str_to_num<int64_t>(pgval_to_string(val, len, format));
        case INT2OID:
            return (int64_t)pgval_to_int16(val, len, format);
        case INT4OID:
            return (int64_t)pgval_to_int32(val, len, format);
        case INT8OID:
            return pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return (int64_t)pgval_to_float(val, len, format);
        case FLOAT8OID:
            return (int64_t)pgval_to_double(val, len, format);
        case NUMERICOID:
            return str_to_num<int16_t>(
                val_from_pgparam<std::string>(oid, val, len, format)
            );

        case TIMEOID:
            return pgval_to_time(val, len, format).pgticks();
        case TIMETZOID:
            return pgval_to_time_tz(val, len, format).pgticks();
        case TIMESTAMPOID:
            return pgval_to_timestamp(val, len, format).pgticks();
        case TIMESTAMPTZOID:
            return pgval_to_timestamp_tz(val, len, format).pgticks();
        case DATEOID:
            return pgval_to_date(val, len, format).pgticks();
        case INTERVALOID:
            return pgval_to_interval(val, len, format).pgticks();

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
float val_from_pgparam<float>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<float>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? 1 : 0;
        case TEXTOID:
            return str_to_num<float>(pgval_to_string(val, len, format));
        case INT2OID:
            return (float)pgval_to_int16(val, len, format);
        case INT4OID:
            return (float)pgval_to_int32(val, len, format);
        case INT8OID:
            return (float)pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return pgval_to_float(val, len, format);
        case FLOAT8OID:
            return (float)pgval_to_double(val, len, format);
        case NUMERICOID:
            return str_to_num<float>(
                val_from_pgparam<std::string>(oid, val, len, format)
            );

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}


template<>
double val_from_pgparam<double>(int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::str_to_num<double>(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case BOOLOID:
            return pgval_to_bool(val, len, format) ? 1 : 0;
        case TEXTOID:
            return str_to_num<double>(pgval_to_string(val, len, format));
        case INT2OID:
            return (double)pgval_to_int16(val, len, format);
        case INT4OID:
            return (double)pgval_to_int32(val, len, format);
        case INT8OID:
            return (double)pgval_to_int64(val, len, format);
        case FLOAT4OID:
            return (double)pgval_to_float(val, len, format);
        case FLOAT8OID:
            return pgval_to_double(val, len, format);
        case NUMERICOID:
            return str_to_num<double>(
                val_from_pgparam<std::string>(oid, val, len, format)
            );

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::time val_from_pgparam<pq_async::time>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::time::parse(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case TEXTOID:
            return pq_async::time::parse(pgval_to_string(val, len, format));
        case INT2OID:
            return pq_async::time((int64_t)pgval_to_int16(val, len, format));
        case INT4OID:
            return pq_async::time((int64_t)pgval_to_int32(val, len, format));
        case INT8OID:
            return pq_async::time(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return pq_async::time((int64_t)pgval_to_float(val, len, format));
        case FLOAT8OID:
            return pq_async::time((int64_t)pgval_to_double(val, len, format));

        case TIMEOID:
            return pgval_to_time(val, len, format);
        // case TIMETZOID:
        // 	return (pq_async::time)pgval_to_time_tz(val, len, format);
        // case TIMESTAMPOID:
        // 	return (pq_async::time)pgval_to_timestamp(val, len, format);
        // case TIMESTAMPTZOID:
        // 	return (pq_async::time)pgval_to_timestamp_tz(val, len, format);
        // case DATEOID:
        // 	return (pq_async::time)pgval_to_date(val, len, format);
        // case INTERVALOID:
        // 	return (pq_async::time)pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::time_tz val_from_pgparam<pq_async::time_tz>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::time_tz::parse(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case TEXTOID:
            return pq_async::time_tz::parse(pgval_to_string(val, len, format));
        case INT2OID:
            return pq_async::time_tz((int64_t)pgval_to_int16(val, len, format));
        case INT4OID:
            return pq_async::time_tz((int64_t)pgval_to_int32(val, len, format));
        case INT8OID:
            return pq_async::time_tz(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return pq_async::time_tz((int64_t)pgval_to_float(val, len, format));
        case FLOAT8OID:
            return pq_async::time_tz(
                (int64_t)pgval_to_double(val, len, format)
            );

        // case TIMEOID:
        // 	return (pq_async::time_tz)pgval_to_time(val, len, format);
        case TIMETZOID:
            return pgval_to_time_tz(val, len, format);
        // case TIMESTAMPOID:
        // 	return (pq_async::time_tz)pgval_to_timestamp(val, len, format);
        // case TIMESTAMPTZOID:
        // 	return (pq_async::time_tz)pgval_to_timestamp_tz(val, len, format);
        // case DATEOID:
        // 	return (pq_async::time_tz)pgval_to_date(val, len, format);
        // case INTERVALOID:
        // 	return (pq_async::time_tz)pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}


template<>
pq_async::timestamp val_from_pgparam<pq_async::timestamp>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::timestamp::parse(std::string(val, len));

    switch(oid){
        case TEXTOID:
            return pq_async::timestamp::parse(
                pgval_to_string(val, len, format)
            );
        case INT2OID:
            return pq_async::timestamp(
                (int64_t)pgval_to_int16(val, len, format)
            );
        case INT4OID:
            return pq_async::timestamp(
                (int64_t)pgval_to_int32(val, len, format)
            );
        case INT8OID:
            return pq_async::timestamp(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return pq_async::timestamp(
                (int64_t)pgval_to_float(val, len, format)
            );
        case FLOAT8OID:
            return pq_async::timestamp(
                (int64_t)pgval_to_double(val, len, format)
            );

        // case TIMEOID:
        // 	return (pq_async::timestamp)pgval_to_time(
        // 		val, len, format
        // 	);
        // case TIMETZOID:
        // 	return (pq_async::timestamp)pgval_to_time_tz(
        // 		val, len, format
        // 	);
        case TIMESTAMPOID:
            return pgval_to_timestamp(
                val, len, format
            );
        case TIMESTAMPTZOID:
            return (pq_async::timestamp)pgval_to_timestamp_tz(
                val, len, format
            );
        // case DATEOID:
        // 	return (pq_async::timestamp)pgval_to_date(val, len, format);
        // case INTERVALOID:
        // 	return (pq_async::timestamp)pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::timestamp_tz val_from_pgparam<pq_async::timestamp_tz>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::timestamp_tz::parse(std::string(val, len));

    switch(oid){
        case TEXTOID:
            return pq_async::timestamp_tz::parse(
                pgval_to_string(val, len, format)
            );
        case INT2OID:
            return pq_async::timestamp_tz(
                (int64_t)pgval_to_int16(val, len, format)
            );
        case INT4OID:
            return pq_async::timestamp_tz(
                (int64_t)pgval_to_int32(val, len, format)
            );
        case INT8OID:
            return pq_async::timestamp_tz(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return pq_async::timestamp_tz(
                (int64_t)pgval_to_float(val, len, format)
            );
        case FLOAT8OID:
            return pq_async::timestamp_tz(
                (int64_t)pgval_to_double(val, len, format)
            );

        // case TIMEOID:
        // 	return (pq_async::timestamp_tz)pgval_to_time(
        // 		val, len, format
        // 	);
        // case TIMETZOID:
        // 	return (pq_async::timestamp_tz)pgval_to_time_tz(
        // 		val, len, format
        // 	);
        case TIMESTAMPOID:
            return (pq_async::timestamp_tz)pgval_to_timestamp(
                val, len, format
            );
        case TIMESTAMPTZOID:
            return pgval_to_timestamp_tz(
                val, len, format
            );
        // case DATEOID:
        // 	return (pq_async::timestamp_tz)pgval_to_date(val, len, format);
        // case INTERVALOID:
        // 	return (pq_async::timestamp_tz)pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}


template<>
pq_async::date val_from_pgparam<pq_async::date>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::date::parse(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case TEXTOID:
            return pq_async::date::parse(pgval_to_string(val, len, format));
        case INT2OID:
            return pq_async::date((int64_t)pgval_to_int16(val, len, format));
        case INT4OID:
            return pq_async::date((int64_t)pgval_to_int32(val, len, format));
        case INT8OID:
            return pq_async::date(pgval_to_int64(val, len, format));
        case FLOAT4OID:
            return pq_async::date((int64_t)pgval_to_float(val, len, format));
        case FLOAT8OID:
            return pq_async::date((int64_t)pgval_to_double(val, len, format));

        // case TIMEOID:
        // 	return (pq_async::date)pgval_to_time(
        // 		val, len, format
        // 	);
        // case TIMETZOID:
        // 	return (pq_async::date)pgval_to_time_tz(
        // 		val, len, format
        // 	);
        // case TIMESTAMPOID:
        // 	return (pq_async::date)pgval_to_timestamp(
        // 		val, len, format
        // 	);
        // case TIMESTAMPTZOID:
        // 	return (pq_async::date)pgval_to_timestamp_tz(
        // 		val, len, format
        // 	);
        case DATEOID:
            return pgval_to_date(val, len, format);
        // case INTERVALOID:
        // 	return (pq_async::date)pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::interval val_from_pgparam<pq_async::interval>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::interval::parse(std::string(val, len));

    // else result is in bin format.
    switch(oid){
        case TEXTOID:
            return pq_async::interval::parse(
                pgval_to_string(val, len, format)
            );
        case INT2OID:
            return pq_async::interval(
                (int64_t)pgval_to_int16(val, len, format),
                0, 0
            );
        case INT4OID:
            return pq_async::interval(
                (int64_t)pgval_to_int32(val, len, format),
                0, 0
            );
        case INT8OID:
            return pq_async::interval(
                pgval_to_int64(val, len, format),
                0, 0
            );
        case FLOAT4OID:
            return pq_async::interval(
                (int64_t)pgval_to_float(val, len, format),
                0, 0
            );
        case FLOAT8OID:
            return pq_async::interval(
                (int64_t)pgval_to_double(val, len, format),
                0, 0
            );

        // case TIMEOID:
        // 	return (pq_async::interval)pgval_to_time(
        // 		val, len, format
        // 	);
        // case TIMETZOID:
        // 	return (pq_async::interval)pgval_to_time_tz(
        // 		val, len, format
        // 	);
        // case TIMESTAMPOID:
        // 	return (pq_async::interval)pgval_to_timestamp(
        // 		val, len, format
        // 	);
        // case TIMESTAMPTZOID:
        // 	return (pq_async::interval)pgval_to_timestamp_tz(
        // 		val, len, format
        // 	);
        // case DATEOID:
        // 	return (pq_async::interval)pgval_to_date(val, len, format);
        case INTERVALOID:
            return pgval_to_interval(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
std::vector<int8_t> val_from_pgparam<std::vector<int8_t>>(
    int oid, char* val, int len, int format)
{
    if(!format){
        //TODO: add support for string format
        throw pq_async::exception("Unsupported format!");
    }

    switch(oid){
        case BYTEAOID:
            return pgval_to_bytea(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::uuid val_from_pgparam<pq_async::uuid>(
    int oid, char* val, int len, int format)
{
    if(!format){
        //TODO: add support for string format
        throw pq_async::exception("Unsupported format!");
    }

    switch(oid){
        case UUIDOID:
            return pgval_to_uuid(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::oid val_from_pgparam<pq_async::oid>(
    int oid, char* val, int len, int format)
{
    if(!format){
        //TODO: add support for string format
        throw pq_async::exception("Unsupported format!");
    }

    switch(oid){
        case OIDOID:
            return pgval_to_oid(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}



template<>
pq_async::cidr val_from_pgparam<pq_async::cidr>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::cidr(std::string(val, len).c_str());

    switch(oid){
        case CIDROID:
            return pgval_to_cidr(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::inet val_from_pgparam<pq_async::inet>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::inet(std::string(val, len).c_str());

    switch(oid){
        case INETOID:
            return pgval_to_inet(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::macaddr val_from_pgparam<pq_async::macaddr>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::macaddr(std::string(val, len).c_str());

    switch(oid){
        case MACADDROID:
            return pgval_to_macaddr(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::macaddr8 val_from_pgparam<pq_async::macaddr8>(
    int oid, char* val, int len, int format)
{
    if(!format)
        return pq_async::macaddr8(std::string(val, len).c_str());

    switch(oid){
        case MACADDR8OID:
            return pgval_to_macaddr8(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::point val_from_pgparam<pq_async::point>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::point(std::string(val, len).c_str());

    switch(oid){
        case POINTOID:
            return pgval_to_point(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::line val_from_pgparam<pq_async::line>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::line(std::string(val, len).c_str());

    switch(oid){
        case LINEOID:
            return pgval_to_line(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::lseg val_from_pgparam<pq_async::lseg>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::lseg(std::string(val, len).c_str());

    switch(oid){
        case LSEGOID:
            return pgval_to_lseg(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::box val_from_pgparam<pq_async::box>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::box(std::string(val, len).c_str());

    switch(oid){
        case BOXOID:
            return pgval_to_box(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::path val_from_pgparam<pq_async::path>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::path(std::string(val, len).c_str());

    switch(oid){
        case PATHOID:
            return pgval_to_path(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::polygon val_from_pgparam<pq_async::polygon>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::polygon(std::string(val, len).c_str());

    switch(oid){
        case POLYGONOID:
            return pgval_to_polygon(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}
template<>
pq_async::circle val_from_pgparam<pq_async::circle>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::circle(std::string(val, len).c_str());

    switch(oid){
        case CIRCLEOID:
            return pgval_to_circle(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::int4range val_from_pgparam<pq_async::int4range>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::int4range(std::string(val, len).c_str());

    switch(oid){
        case INT4RANGEOID:
            return pgval_to_range<pq_async::int4range>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::int8range val_from_pgparam<pq_async::int8range>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::int8range(std::string(val, len).c_str());

    switch(oid){
        case INT8RANGEOID:
            return pgval_to_range<pq_async::int8range>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::numrange val_from_pgparam<pq_async::numrange>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::numrange(std::string(val, len).c_str());

    switch(oid){
        case NUMRANGEOID:
            return pgval_to_range<pq_async::numrange>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::tsrange val_from_pgparam<pq_async::tsrange>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::tsrange(std::string(val, len).c_str());

    switch(oid){
        case TSRANGEOID:
            return pgval_to_range<pq_async::tsrange>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::tstzrange val_from_pgparam<pq_async::tstzrange>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::tstzrange(std::string(val, len).c_str());

    switch(oid){
        case TSTZRANGEOID:
            return pgval_to_range<pq_async::tstzrange>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}

template<>
pq_async::daterange val_from_pgparam<pq_async::daterange>(
    int oid, char* val, int len, int format
)
{
    if(!format)
        return pq_async::daterange(std::string(val, len).c_str());

    switch(oid){
        case DATERANGEOID:
            return pgval_to_range<pq_async::daterange>(val, len, format);

        default:
            throw pq_async::exception("Unsupported oid convertion.");
    }
}



} // namespace pq_async
