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

#include "data_table.h"

namespace pq_async{

data_table::data_table()
{
    pq_async_log_trace("ptr: %p", this);
    _cols.reset(new data_columns_container());
}

data_table::~data_table()
{
    pq_async_log_trace("ptr: %p", this);
}


pq_async::sp_data_value pq_async::data_table::get_value(
    uint32_t row_idx, uint32_t col_id) const
{
    return get_row_safe(row_idx)->get_value(col_id);
}

pq_async::sp_data_value pq_async::data_table::get_value(
    uint32_t row_idx, const char* col_name) const
{
    return get_row_safe(row_idx)->get_value(col_name);
}

void pq_async::data_table::to_json(pq_async::json& rows) const
{
    int count = this->size();

    for(int i = 0; i < count; ++i){
        sp_data_row row = (*this)[i];
        pq_async::json row_obj = pq_async::json::object();
        row->to_json(row_obj);
        rows.push_back(row_obj);
    }
}

void pq_async::data_table::to_json_string(
    std::string& str, const unsigned int current_indent) const
{
    pq_async::json rows = pq_async::json::array();
    to_json(rows);
    str.append(rows.dump(current_indent));
}

} //namespace pq_async