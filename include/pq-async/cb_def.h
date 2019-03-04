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