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

#ifndef _libpq_async_utils_h
#define _libpq_async_utils_h

#include <string>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>
#include <numeric>
#include <type_traits>
#include <functional>
#include <chrono>

#include "traits.h"

namespace pq_async{

void swap2(const int16_t* inp, int16_t* outp, bool to_network);
void swap4(const int32_t* inp, int32_t* outp, bool to_network);
void swap8(const int64_t* inp, int64_t* outp, bool to_network);

} //namespace pq_async

#endif //_libpq_async_utils_h