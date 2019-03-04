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

#include "async.h"
#include "event_queue.h"
#include "event_strand.h"

namespace pq_async {
    
    std::atomic<uint64_t> m_next_task_id(10UL);
    uint64_t get_event_task_id()
    {
        return ++m_next_task_id;
    }
    
    
    void event_task_base::switch_owner(event_queue* new_owner, bool requeue)
    {
        if(!_owner || !new_owner)
            throw pq_async::exception("Owner can't be NULL");
        
        if(requeue)
            _owner->requeue_task(new_owner, this);
        else
            _owner = new_owner;
    }
    
    // initialize the default event_queue
    event_queue* event_queue::_default = new event_queue();
    
    void event_queue::series(
    std::vector< async_series_cb > cbs, async_cb end_cb)
    {
        async::series(this, cbs, end_cb);
    }

    
    
    uint64_t _event_queue_push_back(
        event_queue* eq, sp_event_task tp_task)
    {
        if(tp_task->owner() != eq)
            throw pq_async::exception(
                "Can't requeue a task created on another event_queue!\n"
                "Call the event_task::switch_owner function instead."
            );
        
        if(tp_task->force_push()){
            eq->_tasks.emplace_back(tp_task);
            return tp_task->id();
        }
        
        auto it = std::find_if(
            eq->_tasks.begin(),
            eq->_tasks.end(),
            [task_id=tp_task->id()](const sp_event_task& t)-> bool {
                return t->id() == task_id;
            }
        );
        
        if(it != eq->_tasks.end())
            return tp_task->id();
        eq->_tasks.emplace_back(tp_task);
        return tp_task->id();
    }
    
    uint64_t _event_queue_push_front(
        event_queue* eq, sp_event_task tp_task)
    {
        if(tp_task->owner() != eq)
            throw pq_async::exception(
                "Can't requeue a task created on another event_queue!\n"
                "Call the event_task::switch_owner function instead."
            );
        
        if(tp_task->force_push()){
            eq->_tasks.emplace_front(tp_task);
            return tp_task->id();
        }
        
        auto it = std::find_if(
            eq->_tasks.begin(),
            eq->_tasks.end(),
            [task_id=tp_task->id()](const sp_event_task& t)-> bool {
                return t->id() == task_id;
            }
        );
        
        if(it != eq->_tasks.end())
            return tp_task->id();
        eq->_tasks.emplace_front(tp_task);
        return tp_task->id();
    }
    
    
} //namespace pq_async