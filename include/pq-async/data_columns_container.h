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

#ifndef _libpq_async_data_columns_container_h
#define _libpq_async_data_columns_container_h


#include "data_common.h"
#include "data_column.h"

namespace pq_async{

class data_columns_container;
typedef std::shared_ptr< pq_async::data_columns_container > 
    sp_data_columns_container;

class data_columns_container
    : public std::vector<sp_data_column>
{
public:

    data_columns_container();
    virtual ~data_columns_container();

    sp_data_column get_col(int idx);
    sp_data_column get_col(const char* col_name);
    
    int32_t get_col_index(const char* col_name) const
    {
        std::string tmp_name = md::lower_case_copy(col_name);

        for(unsigned int i = 0; i < this->size(); ++i)
            if(strcmp((*this)[i]->get_cname(), tmp_name.c_str()) == 0)
                return (int)i;
        
        return -1;
    }


private:

};

} //namespace pq_async

#endif //_libpq_async_data_columns_container_h
