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

#include "exceptions.h"

namespace pq_async{

    exception::exception(const std::string& message)
        : std::runtime_error(message.c_str()) { }

    exception::exception(const char* message)
        : std::runtime_error(message) { }

    exception::~exception(){}
    
    connection_pool_assign_exception::connection_pool_assign_exception(const std::string& message)
        : pq_async::exception(message) { }

    connection_pool_assign_exception::connection_pool_assign_exception(const char* message)
        : pq_async::exception(message) { }
    
    connection_pool_assign_exception::~connection_pool_assign_exception(){}
    
    
    cb_error cb_error::no_err;


    std::ostream& operator<<(std::ostream& s, const cb_error& v)
    {
        s << std::string(v.c_str());
        return s;
    }


} //namespace pq_async
