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

#ifndef _libpq_async_traits_h
#define _libpq_async_traits_h

#include <type_traits>
#include <functional>

#ifndef __cpp_lib_transformation_trait_aliases
namespace std{
    template< class T >
    using remove_cv_t       = typename std::remove_cv<T>::type;

    template< class T >
    using remove_const_t    = typename std::remove_const<T>::type;

    template< class T >
    using remove_volatile_t = typename std::remove_volatile<T>::type;
}
#endif

namespace std{
#if __cplusplus <= 201103L
    // template< std::size_t Len, std::size_t Align = /*default-alignment*/ >
    // using aligned_storage_t = typename aligned_storage<Len, Align>::type;
    template<size_t _Len, size_t _Align =
        __alignof__(typename __aligned_storage_msa<_Len>::__type)>
        using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;

    template< bool B, class T = void >
    using enable_if_t = typename enable_if<B,T>::type;

    template< class T >
    using decay_t = typename decay<T>::type;
#endif
#if __cplusplus <= 201402L
    
    template <typename F, typename... Args>
    struct is_invocable :
        std::is_constructible<
            std::function<void(Args ...)>,
            std::reference_wrapper<typename std::remove_reference<F>::type>
        >
    {
    };

    template <typename R, typename F, typename... Args>
    struct is_invocable_r :
        std::is_constructible<
            std::function<R(Args ...)>,
            std::reference_wrapper<typename std::remove_reference<F>::type>
        >
    {
    };
}
#endif

namespace pq_async{

template <class... Args>
struct select_last
{
};
template <typename T>
struct select_last<T>
{
    using type = T;
};
template <class T, class... Args>
struct select_last<T, Args...>
{
    using type = typename select_last<Args...>::type;
};


}// ns: pq_async



#endif //_libpq_async_traits_h
