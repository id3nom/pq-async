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

#ifndef _libpq_async_column_h
#define _libpq_async_column_h


#include "data_common.h"

namespace pq_async{

class data_column;
typedef std::shared_ptr< pq_async::data_column > sp_data_column;

class data_column
{
public:
    data_column(int oid, int index, const std::string& name, int fmt);
    
    virtual ~data_column();

    int get_oid() const { return _oid;}
    int get_index() const { return _index;}
    std::string get_name() const { return _name;}
    const char* get_cname() const { return _name.c_str();}
    int get_format() { return _fmt; }

private:
    int _oid;
    int _index;
    std::string _name;
    int _fmt;
};

} //namespace pq_async

#endif //_libpq_async_column_h
