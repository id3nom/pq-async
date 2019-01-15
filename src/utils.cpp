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
