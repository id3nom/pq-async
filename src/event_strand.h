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

#ifndef _libpq_async_event_strand_h
#define _libpq_async_event_strand_h

#include "data_common.h"
#include "event_queue.h"

namespace pq_async{

template<typename T = int>
class event_strand
    : public event_queue, public event_task_base, 
    public std::enable_shared_from_this< event_strand<T> >
{
    friend class pq_async::event_queue;

    template< typename Task >
    friend uint64_t _event_strand_push_back(event_queue* eq, Task task);
    friend uint64_t _event_strand_push_back(
        event_queue* eq, sp_event_task task
    );

    template< typename Task >
    friend uint64_t _event_strand_push_front(event_queue* eq, Task task);
    friend uint64_t _event_strand_push_front(
        event_queue* eq, sp_event_task task
    );
    
public:
    event_strand(bool auto_requeue = true)
        : event_task_base(event_queue::get_default()),
        _auto_requeue(auto_requeue)
    {
    }
    
    event_strand(event_queue* owner, bool auto_requeue = true)
        : event_task_base(owner), _auto_requeue(auto_requeue)
    {
    }
    
    virtual ~event_strand()
    {
        for(size_t i = 0; i < _tasks.size(); ++i){
            _tasks[i]->_owner = this->_owner;
            this->_owner->push_back(_tasks[i]);
        }
    }
    
    virtual bool force_push(){ return true;}
    virtual size_t size() const
    {
        return _tasks.size();
    }
    
    virtual event_requeue_pos requeue()
    {
        if(_auto_requeue && this->size() > 0)
            return event_requeue_pos::back;
        
        return event_requeue_pos::none;
    }
    
    T data(){ return _data;}
    void data(T val){ _data = val;}
    
    template< typename Task >
    uint64_t push_back(Task task)
    {
        PQ_ASYNC_LOCK_EVENT_QUEUE;
        
        auto id = _event_queue_push_back(this, task);
        
        if(_auto_requeue)
            this->_owner->push_back(
                PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
            );
        return id;
    }	
    
    template< typename Task >
    uint64_t push_front(Task task)
    {
        PQ_ASYNC_LOCK_EVENT_QUEUE;

        auto id = _event_queue_push_front(this, task);
        if(_auto_requeue)
            this->_owner->push_back(
                PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
            );
        return id;
    }
    
    virtual void run_task()
    {
        this->run_n();
    }
    
    void requeue_self_back()
    {
        this->_owner->push_back(
            PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
        );
    }
    
    void requeue_self_front()
    {
        this->_owner->push_front(
            PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
        );
    }
    
private:
    #ifdef PQ_ASYNC_THREAD_SAFE
    mutable std::mutex _mutex;
    #endif

    bool _auto_requeue;
    T _data;
};





/*
template <>
uint64_t event_strand::push_back(sp_event_task tp_task)
{
    _evq_lock lock(_mutex, _is_thread_safe.load(std::memory_order_acquire));
    
    if(tp_task->_owner != this)
        throw pq_async::exception(
            "Can't requeue a task created on another event_queue!\n"
            "Call the event_task::switch_owner function instead."
        );
    
    if(tp_task->force_push()){
        _tasks.emplace_back(tp_task);
        this->_owner->push_back(
            PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
        );
        return tp_task->id();
    }
    
    auto it = std::find_if(
        _tasks.begin(),
        _tasks.end(),
        [task_id=tp_task->id()](const sp_event_task& t)-> bool {
            return t->id() == task_id;
        }
    );
    
    if(it != _tasks.end())
        return tp_task->id();
    _tasks.emplace_back(tp_task);
    this->_owner->push_back(
        PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
    );
    return tp_task->id();
}

template <>
uint64_t event_strand::push_front(sp_event_task tp_task)
{
    _evq_lock lock(_mutex, _is_thread_safe.load(std::memory_order_acquire));

    if(tp_task->_owner != this)
        throw pq_async::exception(
            "Can't requeue a task created on another event_queue!\n"
            "Call the event_task::switch_owner function instead."
        );
    
    if(tp_task->force_push()){
        _tasks.emplace_back(tp_task);
        this->_owner->push_back(
            PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
        );
        return tp_task->id();
    }
        
    auto it = std::find_if(
        _tasks.begin(),
        _tasks.end(),
        [task_id=tp_task->id()](const sp_event_task& t)-> bool {
            return t->id() == task_id;
        }
    );
    
    if(it != _tasks.end())
        return tp_task->id();
    _tasks.emplace_front(tp_task);
    this->_owner->push_back(
        PQ_ASYNC_TO_TASKBASE(this->shared_from_this())
    );
    return tp_task->id();
}
*/




} //namespace pq_async
#endif //_libpq_async_event_strand_h
