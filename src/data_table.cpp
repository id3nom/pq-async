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

#include "data_table_t.h"

namespace pq_async{

data_table_t::data_table_t()
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
    _cols.reset(new data_columns_container_t());
}

data_table_t::~data_table_t()
{
    PQ_ASYNC_DEF_TRACE("ptr: {:p}", (void*)this);
}


pq_async::data_value pq_async::data_table_t::get_value(
    uint32_t row_idx, uint32_t col_id) const
{
    return get_row_safe(row_idx)->get_value(col_id);
}

pq_async::data_value pq_async::data_table_t::get_value(
    uint32_t row_idx, const char* col_name) const
{
    return get_row_safe(row_idx)->get_value(col_name);
}

void pq_async::data_table_t::to_json(pq_async::json& rows) const
{
    int count = this->size();

    for(int i = 0; i < count; ++i){
        data_row row = (*this)[i];
        pq_async::json row_obj = pq_async::json::object();
        row->to_json(row_obj);
        rows.push_back(row_obj);
    }
}

void pq_async::data_table_t::to_json_string(
    std::string& str, const unsigned int current_indent) const
{
    pq_async::json rows = pq_async::json::array();
    to_json(rows);
    str.append(rows.dump(current_indent));
}

} //namespace pq_async