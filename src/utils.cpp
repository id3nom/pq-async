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

#include "utils.h"

#include <arpa/inet.h>
#include <cstring>

namespace pq_async{

constexpr char hexmap[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

std::string hex_to_str(const uint8_t* data, int len)
{
    std::string s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
    }
    return s;
}

void swap2(const int16_t* inp, int16_t* outp, bool to_network)
{
    static int32_t n = 1;
    
    if( *(char *)&n == 1)
        *outp = to_network ? htons( *inp) : ntohs( *inp);
    else
        *outp = *inp;
}

void swap4(const int32_t* inp, int32_t* outp, bool to_network)
{
    static int32_t n = 1;
    
    if( *(char *)&n == 1)
        *outp = to_network ? htonl( *inp) : ntohl( *inp);
    else
        *outp = *inp;
}

void swap8(const int64_t* inp, int64_t* outp, bool to_network)
{
    static int32_t n = 1;
    uint32_t in[2];
    uint32_t out[2];
    memcpy(&in, inp, 8);
    
    if (*(char *)&n == 1){
        out[0] = (uint32_t) (to_network ? htonl(in[1]) : ntohl(in[1]));
        out[1] = (uint32_t) (to_network ? htonl(in[0]) : ntohl(in[0]));
    } else {
        out[0] = in[0];
        out[1] = in[1];
    }

    memcpy(outp, out, 8);
}

} //namespace ps_async
