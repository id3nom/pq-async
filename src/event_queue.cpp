/*
    This file is part of pq-async
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