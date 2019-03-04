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

#ifndef _libpq_async_async_h
#define _libpq_async_async_h

#include <type_traits>
#include <functional>
#include <vector>
#include "exceptions.h"
#include "cb_def.h"
#include "event_strand.h"

namespace pq_async{


class async final
{
    async(){}
public:

    template<
        typename Iterator,
        typename T,
        typename std::enable_if<
            std::is_invocable_r<
                void,
                T,
                const typename std::iterator_traits<Iterator>::value_type&,
                std::function<void(const cb_error& err)>
            >::value
        , int32_t>::type = -1
    >
    static void each(
        event_queue* eq,
        Iterator first, Iterator last,
        T cb,
        async_cb end_cb)
    {
        typedef typename std::iterator_traits<Iterator>::value_type U;
        async_item_cb<U> aicb = cb;
        async::each<Iterator,U>(eq, first, last, aicb, end_cb);
    }

    
    template<
        typename Iterator,
        typename T = typename std::iterator_traits<Iterator>::value_type
    >
    static void each(
        event_queue* eq,
        Iterator first, Iterator last,
        async_item_cb<T> cb,
        async_cb end_cb)
    {
        auto it = first;
        //typename Container::const_iterator it = elements.begin();
        if(it == last){
            eq->push_back(std::bind(end_cb, nullptr));
            return;
        }
        
        eq->push_back(
            std::bind(
                each_iter<Iterator, T>, 
                eq, last, it, cb, end_cb
            )
        );
        
        // std::function<void()> iter = 
        // [iter, last, it, eq, cb, end_cb]() mutable ->void{
        // 	cb((*it),[iter, last, it, eq, end_cb]
        // 	(const cb_error& err) mutable -> void {
        // 		if(err){
        // 			eq->push_back(std::bind(end_cb, err));
        // 			return;
        // 		}
        //		
        // 		if(++it != last)
        // 			eq->push_back(iter);
        // 		else
        // 			eq->push_back(std::bind(end_cb, nullptr));
        // 	});
        // };
        //
        // eq->push_back(iter);
    }

    /*!
     * 
     *	example: 
     *		pq_async::async::series({
     *			[&](pq_async::async_cb scb) -> void {
     *				return scb(nullptr);
     *			},
     *			[&](pq_async::async_cb scb) -> void {
     *				return scb(nullptr);
     *			},
     *		}, [&](const pq_async::cb_error& err) -> void {
     *			if(err)
     *				return cb(err, cfg);
     *			cb(pq_async::cb_error::no_err, cfg);
     *		});
     */
    static void series(
        event_queue* eq,
        std::vector< async_series_cb > cbs, async_cb end_cb)
    {
        if(cbs.size() == 0){
            eq->push_back(std::bind(end_cb, nullptr));
            return;
        }
        
        auto strand = eq->new_strand<pq_async::cb_error>(false);
        // sp_thread_pool_task_group<pq_async::cb_error> strand(
        //	new pq_async::thread_pool_task_group<pq_async::cb_error>(nullptr));
        // strand->set_requeue_mode(false);
        
        for(auto i = 0U; i < cbs.size(); ++i){
            strand->push_back([strand, cb = cbs[i]]() -> void {
                if(strand->data()){
                    strand->requeue_self_back();
                    return;
                }
                
                auto cb_called = std::make_shared<bool>(false);
                cb((async_cb)[strand, cb_called]
                (const pq_async::cb_error& err) -> void {
                    if(*cb_called)
                        pq_async_log_fatal("Callback already called once!");
                    *cb_called = true;
                    if(err)
                        strand->data(err);
                    
                    strand->requeue_self_back();
                });
            });
        }
        strand->push_back([strand, end_cb]() -> void {
            end_cb(strand->data());
            strand->requeue_self_back();
        });
        strand->requeue_self_back();
    }
    
private:

    template<
        typename Iterator,
        typename T = typename std::iterator_traits<Iterator>::value_type
    >
    static void each_iter(
        event_queue* eq,
        Iterator last, Iterator it,
        async_item_cb<T> cb,
        async_cb end_cb)
    {
        cb(
            (*it),
            std::bind(
                each_iter_cb<Iterator, T>,
                eq, last, it, cb, end_cb,
                std::placeholders::_1
            )
        );
    }
    
    template<
        typename Iterator,
        typename T = typename std::iterator_traits<Iterator>::value_type
    >
    static void each_iter_cb(
        event_queue* eq,
        Iterator last, Iterator it,
        async_item_cb<T> cb,
        async_cb end_cb,
        const cb_error& err)
    {
        if(err){
            eq->push_back(std::bind(end_cb, err));
            return;
        }
        
        if(++it != last)
            eq->push_back(
                std::bind(
                    each_iter<Iterator, T>, 
                    eq, last, it, cb, end_cb
                )
            );
        else
            eq->push_back(std::bind(end_cb, nullptr));
    }
    
};

// template<
// 	typename Iterator,
// 	typename T
// >
// void event_queue::each(
// 	Iterator first, Iterator last,
// 	async_item_cb<T> cb,
// 	async_cb end_cb)
// {
// 	async::each<Iterator, T>(this, first, last, cb, end_cb);
// }

template<
    typename Iterator,
    typename T
>
void event_queue::each(
    Iterator first, Iterator last,
    T cb,
    async_cb end_cb)
{
    typedef typename std::iterator_traits<Iterator>::value_type U;
    async_item_cb<U> aicb = cb;
    async::each<Iterator,U>(this, first, last, aicb, end_cb);
}


} // ns: pq_async
#endif //_libpq_async_async_h
