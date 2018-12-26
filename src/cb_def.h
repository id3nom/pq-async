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

#ifndef _libpq_async_cb_def_h
#define _libpq_async_cb_def_h

#include <type_traits>
#include <functional>
#include "exceptions.h"
#include "utils.h"
#include "log.h"

namespace pq_async{

typedef std::function<void()> void_cb;
typedef std::function<void(const cb_error& err)> async_cb;
template <typename T>
using value_cb = std::function<void(const cb_error& err, T)>;
template <typename T>
using no_err_value_cb = std::function<void(T)>;


template<typename T>
using async_item_cb = typename std::function<void(const T& val, async_cb)>;
typedef std::function<void(async_cb)> async_series_cb;

void noop_cb(const cb_error& err);
const async_cb run_async = &noop_cb;

template<typename T, typename U, typename V,
    typename std::enable_if<
        std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        !std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            const cb_error&,
            U
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            U
        >::value
    , int32_t>::type = -1
>
void assign_value_cb(
    T& cb, const V& value) noexcept
{
    cb = [value](const cb_error& err, U /*val*/){
        if(err)
            pq_async_log_fatal("Unhandled exception:\n%s", err.c_str());
        value();
    };
}

template<typename T, typename U, typename V,
    typename std::enable_if<
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            const cb_error&,
            U
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            U
        >::value
    , int32_t>::type = -1
>
void assign_value_cb(
    T& cb, const V& value) noexcept
{
    cb = [value](const cb_error& err, U /*val*/){
        value(err);
    };
}

template<typename T, typename U, typename V,
    typename std::enable_if<
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        !std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value &&
        std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            const cb_error&,
            U
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            U
        >::value
    , int32_t>::type = -1
>
void assign_value_cb(
    T& cb, const V& value) noexcept
{
    cb = value;
}

template<typename T, typename U, typename V,
    typename std::enable_if<
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        !std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value &&
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            const cb_error&,
            U
        >::value &&
        std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type,
            U
        >::value
    , int32_t>::type = -1
>
void assign_value_cb(
    T& cb, const V& value) noexcept
{
    cb = [value](const cb_error& err, U val){
        if(err)
            pq_async_log_fatal("Unhandled exception:\n%s", err.c_str());
        value(val);
    };
}




template<typename T, typename V,
    typename std::enable_if<
        std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        !std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value
    , int32_t>::type = -1
>
void assign_async_cb(
    T& cb, const V& value) noexcept
{
    cb = [value](const cb_error& err){
        if(err)
            pq_async_log_fatal("Unhandled exception:\n%s", err.c_str());
        value();
    };
}

template<typename T, typename V,
    typename std::enable_if<
        !std::is_invocable_r<
            void,
            typename std::remove_pointer<V>::type
        >::value &&
        std::is_invocable_r<
            void, 
            typename std::remove_pointer<V>::type,
            const cb_error&
        >::value
    , int32_t>::type = -1
>
void assign_async_cb(
    T& cb, const V& value) noexcept
{
    cb = value;
}



}// ns: pq_async

#endif //_libpq_async_cb_def_h