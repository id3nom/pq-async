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


#define PGSQL_AF_INET	(AF_INET + 0)
#define PGSQL_AF_INET6	(AF_INET + 1)

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cassert>

#include "pg_type_net_def.h"



namespace pq_async{

static char* inet_net_ntop(
    int af, const void *src, int bits, char *dst, size_t size
);
static int inet_net_pton(
    int af, const char *src, void *dst, ssize_t size
);

static bool address_ok(unsigned char *a, int bits, int family);

static int32_t network_cmp_internal(
    net_family a1_f, uint8_t a1_b, const uint8_t* a1_d,
    net_family a2_f, uint8_t a2_b, const uint8_t* a2_d
);

static int32_t net_cmp(const cidr& a1, const cidr& a2)
{
    return network_cmp_internal(
        a1.get_family(), a1.get_mask_bits(), a1.data(),
        a2.get_family(), a2.get_mask_bits(), a2.data()
    );
}

static int32_t net_cmp(const inet& a1, const inet& a2)
{
    return network_cmp_internal(
        a1.get_family(), a1.get_mask_bits(), a1.data(),
        a2.get_family(), a2.get_mask_bits(), a2.data()
    );
}


cidr::cidr(const char* src)
    : _family(net_family::pq_net_family_none), _bits(0), _ipaddr{}
{
    int af;
    if (strchr(src, ':') != NULL)
        af = PGSQL_AF_INET6;
    else
        af = PGSQL_AF_INET;
    
    this->_family = af == PGSQL_AF_INET ?
        net_family::pq_net_family_inet :
        net_family::pq_net_family_inet6;
        
    this->_bits = inet_net_pton(
        af, src, this->_ipaddr,
        af == PGSQL_AF_INET ? 4 : 16
    );
    
    if((this->_bits < 0) || (this->_bits > (PGSQL_AF_INET ? 32 : 128)))
        throw pq_async::exception(
            "invalid input syntax for type cidr"
        );
    
    if (!address_ok(this->_ipaddr, this->_bits, af))
        throw pq_async::exception(
            "invalid external \"cidr\" value, Value has bits set to right of mask."
        );
}


cidr::operator std::string() const
{
    char tmp[
        sizeof(
            "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:255.255.255.255/128"
        )
    ] = {};
    
    char *dst;
    int len;
    
    int af = this->_family == net_family::pq_net_family_inet ?
        PGSQL_AF_INET : PGSQL_AF_INET6;
    
    dst = inet_net_ntop(
        af, this->_ipaddr, this->_bits, tmp, sizeof(tmp)
    );
    
    if (dst == NULL)
        throw pq_async::exception("could not format inet value");

    /* For CIDR, add /n if not present */
    if(strchr(tmp, '/') == NULL){
        len = strlen(tmp);
        snprintf(tmp + len, sizeof(tmp) - len, "/%u", this->_bits);
    }
    
    return std::string(tmp);
}

bool cidr::operator ==(const cidr& b) const
{
    return net_cmp(*this, b) == 0;
}

bool cidr::operator !=(const cidr& b) const
{
    return net_cmp(*this, b) != 0;
}

bool cidr::operator <(const cidr& b) const
{
    return net_cmp(*this, b) < 0;
}

bool cidr::operator >(const cidr& b) const
{
    return net_cmp(*this, b) > 0;
}

bool cidr::operator <=(const cidr& b) const
{
    return net_cmp(*this, b) <= 0;
}

bool cidr::operator >=(const cidr& b) const
{
    return net_cmp(*this, b) >= 0;
}



inet::inet(const char* src)
    : _family(net_family::pq_net_family_none), _bits(0), _ipaddr{}
{
    int af;
    if (strchr(src, ':') != NULL)
        af = PGSQL_AF_INET6;
    else
        af = PGSQL_AF_INET;
    
    this->_family = af == PGSQL_AF_INET ?
        net_family::pq_net_family_inet :
        net_family::pq_net_family_inet6;
    
    this->_bits = inet_net_pton(
        af, src, this->_ipaddr, -1
    );
    
    if((this->_bits < 0) || (this->_bits > (PGSQL_AF_INET ? 32 : 128)))
        throw pq_async::exception(
            "invalid input syntax for type inet"
        );
}

inet::operator std::string() const
{
    char tmp[
        sizeof(
            "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:255.255.255.255/128"
        )
    ] = {};
    
    char *dst;
    
    int af = this->_family == net_family::pq_net_family_inet ?
        PGSQL_AF_INET : PGSQL_AF_INET6;
    
    dst = inet_net_ntop(
        af, this->_ipaddr, this->_bits, tmp, sizeof(tmp)
    );
    
    if (dst == NULL)
        throw pq_async::exception("could not format inet value");
    
    return std::string(tmp);
}

bool inet::operator ==(const inet& b) const
{
    return net_cmp(*this, b) == 0;
}

bool inet::operator !=(const inet& b) const
{
    return net_cmp(*this, b) != 0;
}

bool inet::operator <(const inet& b) const
{
    return net_cmp(*this, b) < 0;
}

bool inet::operator >(const inet& b) const
{
    return net_cmp(*this, b) > 0;
}

bool inet::operator <=(const inet& b) const
{
    return net_cmp(*this, b) <= 0;
}

bool inet::operator >=(const inet& b) const
{
    return net_cmp(*this, b) >= 0;
}





static const signed char hexlookup[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/*
 * hex2_to_uchar - convert 2 hex digits to a byte (unsigned char)
 *
 * This will ereport() if the end of the string is reached ('\0' found), or if
 * either character is not a valid hex digit.
 *
 */
static inline unsigned char
hex2_to_uchar(const unsigned char *ptr)
{
    unsigned char ret = 0;
    signed char lookup;

    /* Handle the first character */
    if (*ptr > 127)
        goto invalid_input;

    lookup = hexlookup[*ptr];
    if (lookup < 0)
        goto invalid_input;

    ret = lookup << 4;

    /* Move to the second character */
    ptr++;

    if (*ptr > 127)
        goto invalid_input;

    lookup = hexlookup[*ptr];
    if (lookup < 0)
        goto invalid_input;

    ret += lookup;

    return ret;

invalid_input:
    throw pq_async::exception(
        "invalid input syntax for type macaddr8"
    );

    /* We do not actually reach here */
    return 0;
}


/*
 *	Utility macros used for sorting and comparing:
 */

#define macaddr_hibits(addr) \
  ((unsigned long)(((addr)->a<<16)|((addr)->b<<8)|((addr)->c)))

#define macaddr_lobits(addr) \
  ((unsigned long)(((addr)->d<<16)|((addr)->e<<8)|((addr)->f)))

#define macaddr8_hibits(addr) \
  ((unsigned long)(((addr)->a<<24) | ((addr)->b<<16) | ((addr)->c<<8) | ((addr)->d)))

#define macaddr8_lobits(addr) \
  ((unsigned long)(((addr)->e<<24) | ((addr)->f<<16) | ((addr)->g<<8) | ((addr)->h)))


/*
 *	Comparison function for sorting:
 */

static int
macaddr_cmp_internal(const macaddr *a1, const macaddr *a2)
{
    if (macaddr_hibits(a1) < macaddr_hibits(a2))
        return -1;
    else if(macaddr_hibits(a1) > macaddr_hibits(a2))
        return 1;
    else if (macaddr_lobits(a1) < macaddr_lobits(a2))
        return -1;
    else if (macaddr_lobits(a1) > macaddr_lobits(a2))
        return 1;
    else
        return 0;
}

/*
 * macaddr8_cmp_internal - comparison function for sorting:
 */
static int32_t
macaddr8_cmp_internal(const macaddr8 *a1, const macaddr8 *a2)
{
    if (macaddr8_hibits(a1) < macaddr8_hibits(a2))
        return -1;
    else if (macaddr8_hibits(a1) > macaddr8_hibits(a2))
        return 1;
    else if (macaddr8_lobits(a1) < macaddr8_lobits(a2))
        return -1;
    else if (macaddr8_lobits(a1) > macaddr8_lobits(a2))
        return 1;
    else
        return 0;
}



macaddr::macaddr(const char* str)
{
    int a, b, c, d, e, f;
    char junk[2];
    int count;
    
    /* %1s matches iff there is trailing non-whitespace garbage */

    count = sscanf(str, "%x:%x:%x:%x:%x:%x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%x-%x-%x-%x-%x-%x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%2x%2x%2x:%2x%2x%2x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%2x%2x%2x-%2x%2x%2x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%2x%2x.%2x%2x.%2x%2x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%2x%2x-%2x%2x-%2x%2x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
    count = sscanf(str, "%2x%2x%2x%2x%2x%2x%1s",
    &a, &b, &c, &d, &e, &f, junk);
    if (count != 6)
        throw pq_async::exception(
            "invalid input syntax for type macaddr"
        );
    
    if ((a < 0) || (a > 255) || (b < 0) || (b > 255) ||
        (c < 0) || (c > 255) || (d < 0) || (d > 255) ||
        (e < 0) || (e > 255) || (f < 0) || (f > 255))
        throw pq_async::exception(
            "invalid octet value in \"macaddr\""
        );
    
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
    this->e = e;
    this->f = f;
}


bool macaddr::operator ==(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) == 0;
}
bool macaddr::operator !=(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) != 0;
}
bool macaddr::operator <(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) < 0;
}
bool macaddr::operator >(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) > 0;
}
bool macaddr::operator <=(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) <= 0;
}
bool macaddr::operator >=(const macaddr& b) const
{
    return macaddr_cmp_internal(this, &b) >= 0;
}



macaddr8::macaddr8(const char* str)
{
    const unsigned char *ptr = (const unsigned char*)str;
    
    unsigned char a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int count = 0;
    unsigned char spacer = '\0';

    /* skip leading spaces */
    while (*ptr && isspace(*ptr))
        ptr++;

    /* digits must always come in pairs */
    while (*ptr && *(ptr + 1))
    {
        /*
         * Attempt to decode each byte, which must be 2 hex digits in a row.
         * If either digit is not hex, hex2_to_uchar will throw ereport() for
         * us.  Either 6 or 8 byte MAC addresses are supported.
         */

        /* Attempt to collect a byte */
        count++;

        switch (count)
        {
            case 1:
                a = hex2_to_uchar(ptr);
                break;
            case 2:
                b = hex2_to_uchar(ptr);
                break;
            case 3:
                c = hex2_to_uchar(ptr);
                break;
            case 4:
                d = hex2_to_uchar(ptr);
                break;
            case 5:
                e = hex2_to_uchar(ptr);
                break;
            case 6:
                f = hex2_to_uchar(ptr);
                break;
            case 7:
                g = hex2_to_uchar(ptr);
                break;
            case 8:
                h = hex2_to_uchar(ptr);
                break;
            default:
                /* must be trailing garbage... */
                throw pq_async::exception(
                    "invalid input syntax for type macaddr8"
                );
        }

        /* Move forward to where the next byte should be */
        ptr += 2;

        /* Check for a spacer, these are valid, anything else is not */
        if (*ptr == ':' || *ptr == '-' || *ptr == '.')
        {
            /* remember the spacer used, if it changes then it isn't valid */
            if (spacer == '\0')
                spacer = *ptr;

            /* Have to use the same spacer throughout */
            else if (spacer != *ptr)
                throw pq_async::exception(
                    "invalid input syntax for type macaddr8"
                );

            /* move past the spacer */
            ptr++;
        }

        /* allow trailing whitespace after if we have 6 or 8 bytes */
        if (count == 6 || count == 8)
        {
            if (isspace(*ptr))
            {
                while (*++ptr && isspace(*ptr));

                /* If we found a space and then non-space, it's invalid */
                if (*ptr)
                    throw pq_async::exception(
                        "invalid input syntax for type macaddr8"
                    );
            }
        }
    }

    /* Convert a 6 byte MAC address to macaddr8 */
    if (count == 6){
        h = f;
        g = e;
        f = d;

        d = 0xFF;
        e = 0xFE;
    } else if (count != 8)
        throw pq_async::exception(
            "invalid input syntax for type macaddr8"
        );

    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
    this->e = e;
    this->f = f;
    this->g = g;
    this->h = h;
}

bool macaddr8::operator ==(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) == 0;
}
bool macaddr8::operator !=(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) != 0;
}
bool macaddr8::operator <(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) < 0;
}
bool macaddr8::operator >(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) > 0;
}
bool macaddr8::operator <=(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) <= 0;
}
bool macaddr8::operator >=(const macaddr8& b) const
{
    return macaddr8_cmp_internal(this, &b) >= 0;
}



std::ostream& operator<<(std::ostream& os, const cidr& v)
{
    os << (std::string)v;
    return os;
}

std::ostream& operator<<(std::ostream& os, const inet& v)
{
    os << (std::string)v;
    return os;
}

std::ostream& operator<<(std::ostream& os, const macaddr& v)
{
    os << (std::string)v;
    return os;
}

std::ostream& operator<<(std::ostream& os, const macaddr8& v)
{
    os << (std::string)v;
    return os;
}

std::istream& operator>>(std::istream& is, cidr& v)
{
    std::string s;
    is >> s;
    v = cidr(s.c_str());
    return is;
}

std::istream& operator>>(std::istream& is, inet& v)
{
    std::string s;
    is >> s;
    v = inet(s.c_str());
    return is;
}

std::istream& operator>>(std::istream& is, macaddr& v)
{
    std::string s;
    is >> s;
    v = macaddr(s.c_str());
    return is;
}

std::istream& operator>>(std::istream& is, macaddr8& v)
{
    std::string s;
    is >> s;
    v = macaddr8(s.c_str());
    return is;
}








/* msb for char */
#define HIGHBIT (0x80)
#define IS_HIGHBIT_SET(ch) ((unsigned char)(ch) & HIGHBIT)

/*
 * int
 * bitncmp(l, r, n)
 *		compare bit masks l and r, for n bits.
 * return:
 *		<0, >0, or 0 in the libc tradition.
 * note:
 *		network byte order assumed.  this means 192.5.5.240/28 has
 *		0x11110000 in its fourth octet.
 * author:
 *		Paul Vixie (ISC), June 1996
 */
static int bitncmp(const unsigned char *l, const unsigned char *r, int n)
{
    unsigned int lb,
                rb;
    int			x,
                b;

    b = n / 8;
    x = memcmp(l, r, b);
    if (x || (n % 8) == 0)
        return x;

    lb = l[b];
    rb = r[b];
    for (b = n % 8; b > 0; b--)
    {
        if (IS_HIGHBIT_SET(lb) != IS_HIGHBIT_SET(rb))
        {
            if (IS_HIGHBIT_SET(lb))
                return 1;
            return -1;
        }
        lb <<= 1;
        rb <<= 1;
    }
    return 0;
}


static int32_t network_cmp_internal(
    net_family a1_f, uint8_t a1_b, const uint8_t* a1_d,
    net_family a2_f, uint8_t a2_b, const uint8_t* a2_d
    )
{
    if((int)a1_f == (int)a2_f){
        int order = bitncmp(
            a1_d, a2_d,
            std::min(a1_b, a2_b)
        );
        
        if (order != 0)
            return order;
        order = ((int)a1_b) - ((int)a2_b);
        if (order != 0)
            return order;
        return bitncmp(
            a1_d, a2_d,
            a1_f == net_family::pq_net_family_inet ? 32 : 128
        );
    }
    
    return
        (a1_f == net_family::pq_net_family_inet ? 
            PGSQL_AF_INET : PGSQL_AF_INET6) - 
        (a2_f == net_family::pq_net_family_inet ? 
            PGSQL_AF_INET : PGSQL_AF_INET6);
}


/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *	  src/port/inet_net_ntop.c
 */

#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
#define NS_INADDRSZ 4

#ifdef SPRINTF_CHAR
#define SPRINTF(x) strlen(sprintf/**/x)
#else
#define SPRINTF(x) ((size_t)sprintf x)
#endif

static char *inet_net_ntop_ipv4(const u_char *src, int bits,
                   char *dst, size_t size);
static char *inet_net_ntop_ipv6(const u_char *src, int bits,
                   char *dst, size_t size);


/*
 * char *
 * inet_net_ntop(af, src, bits, dst, size)
 *	convert host/network address from network to presentation format.
 *	"src"'s size is determined from its "af".
 * return:
 *	pointer to dst, or NULL if an error occurred (check errno).
 * note:
 *	192.5.5.1/28 has a nonzero host part, which means it isn't a network
 *	as called for by inet_net_pton() but it can be a host address with
 *	an included netmask.
 * author:
 *	Paul Vixie (ISC), October 1998
 */
static char *
inet_net_ntop(int af, const void *src, int bits, char *dst, size_t size)
{
    /*
     * We need to cover both the address family constants used by the PG inet
     * type (PGSQL_AF_INET and PGSQL_AF_INET6) and those used by the system
     * libraries (AF_INET and AF_INET6).  We can safely assume PGSQL_AF_INET
     * == AF_INET, but the INET6 constants are very likely to be different. If
     * AF_INET6 isn't defined, silently ignore it.
     */
    switch (af)
    {
        case PGSQL_AF_INET:
            return (inet_net_ntop_ipv4((const u_char*)src, bits, dst, size));
        case PGSQL_AF_INET6:
#if defined(AF_INET6) && AF_INET6 != PGSQL_AF_INET6
        case AF_INET6:
#endif
            return (inet_net_ntop_ipv6((const u_char*)src, bits, dst, size));
        default:
            errno = EAFNOSUPPORT;
            return (NULL);
    }
}

/*
 * static char *
 * inet_net_ntop_ipv4(src, bits, dst, size)
 *	convert IPv4 network address from network to presentation format.
 *	"src"'s size is determined from its "af".
 * return:
 *	pointer to dst, or NULL if an error occurred (check errno).
 * note:
 *	network byte order assumed.  this means 192.5.5.240/28 has
 *	0b11110000 in its fourth octet.
 * author:
 *	Paul Vixie (ISC), October 1998
 */
static char *
inet_net_ntop_ipv4(const u_char *src, int bits, char *dst, size_t size)
{
    char	   *odst = dst;
    char	   *t;
    int			len = 4;
    int			b;

    if (bits < 0 || bits > 32)
    {
        errno = EINVAL;
        return (NULL);
    }

    /* Always format all four octets, regardless of mask length. */
    for (b = len; b > 0; b--)
    {
        if (size <= sizeof ".255")
            goto emsgsize;
        t = dst;
        if (dst != odst)
            *dst++ = '.';
        dst += SPRINTF((dst, "%u", *src++));
        size -= (size_t) (dst - t);
    }

    /* don't print masklen if 32 bits */
    if (bits != 32)
    {
        if (size <= sizeof "/32")
            goto emsgsize;
        dst += SPRINTF((dst, "/%u", bits));
    }

    return (odst);

emsgsize:
    errno = EMSGSIZE;
    return (NULL);
}

static int
decoct(const u_char *src, int bytes, char *dst, size_t size)
{
    char	   *odst = dst;
    char	   *t;
    int			b;

    for (b = 1; b <= bytes; b++)
    {
        if (size <= sizeof "255.")
            return (0);
        t = dst;
        dst += SPRINTF((dst, "%u", *src++));
        if (b != bytes)
        {
            *dst++ = '.';
            *dst = '\0';
        }
        size -= (size_t) (dst - t);
    }
    return (dst - odst);
}

static char *
inet_net_ntop_ipv6(const u_char *src, int bits, char *dst, size_t size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough to
     * contain a value of the specified size.  On some systems, like Crays,
     * there is no such thing as an integer variable with 16 bits. Keep this
     * in mind if you think this function should have been coded to use
     * pointer overlays.  All the world's not a VAX.
     */
    char		tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128"];
    char	   *tp;
    struct
    {
        int			base,
                    len;
    }			best, cur;
    u_int		words[NS_IN6ADDRSZ / NS_INT16SZ];
    int			i;

    if ((bits < -1) || (bits > 128))
    {
        errno = EINVAL;
        return (NULL);
    }

    /*
     * Preprocess: Copy the input (bytewise) array into a wordwise array. Find
     * the longest run of 0x00's in src[] for :: shorthanding.
     */
    memset(words, '\0', sizeof words);
    for (i = 0; i < NS_IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    cur.base = -1;
    best.len = 0;
    cur.len = 0;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        }
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    tp = tmp;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len))
        {
            if (i == best.base)
                *tp++ = ':';
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
            *tp++ = ':';
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 && (best.len == 6 ||
                                         (best.len == 7 && words[7] != 0x0001) ||
                                         (best.len == 5 && words[5] == 0xffff)))
        {
            int			n;

            n = decoct(src + 12, 4, tp, sizeof tmp - (tp - tmp));
            if (n == 0)
            {
                errno = EMSGSIZE;
                return (NULL);
            }
            tp += strlen(tp);
            break;
        }
        tp += SPRINTF((tp, "%x", words[i]));
    }

    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) ==
        (NS_IN6ADDRSZ / NS_INT16SZ))
        *tp++ = ':';
    *tp = '\0';

    if (bits != -1 && bits != 128)
        tp += SPRINTF((tp, "/%u", bits));

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((size_t) (tp - tmp) > size)
    {
        errno = EMSGSIZE;
        return (NULL);
    }
    strcpy(dst, tmp);
    return (dst);
}




static int	inet_net_pton_ipv4(const char *src, u_char *dst);
static int	inet_cidr_pton_ipv4(const char *src, u_char *dst, size_t size);
static int	inet_net_pton_ipv6(const char *src, u_char *dst);
static int	inet_cidr_pton_ipv6(const char *src, u_char *dst, size_t size);


/*
 * int
 * inet_net_pton(af, src, dst, size)
 *	convert network number from presentation to network format.
 *	accepts hex octets, hex strings, decimal octets, and /CIDR.
 *	"size" is in bytes and describes "dst".
 * return:
 *	number of bits, either imputed classfully or specified with /CIDR,
 *	or -1 if some failure occurred (check errno).  ENOENT means it was
 *	not a valid network specification.
 * author:
 *	Paul Vixie (ISC), June 1996
 *
 * Changes:
 *	I added the inet_cidr_pton function (also from Paul) and changed
 *	the names to reflect their current use.
 *
 */
static int inet_net_pton(int af, const char *src, void *dst, ssize_t size)
{
    switch (af)
    {
        case PGSQL_AF_INET:
            return size == -1 ?
                inet_net_pton_ipv4(src, (u_char*)dst) :
                inet_cidr_pton_ipv4(src, (u_char*)dst, size);
        case PGSQL_AF_INET6:
            return size == -1 ?
                inet_net_pton_ipv6(src, (u_char*)dst) :
                inet_cidr_pton_ipv6(src, (u_char*)dst, size);
        default:
            errno = EAFNOSUPPORT;
            return -1;
    }
}

/*
 * static int
 * inet_cidr_pton_ipv4(src, dst, size)
 *	convert IPv4 network number from presentation to network format.
 *	accepts hex octets, hex strings, decimal octets, and /CIDR.
 *	"size" is in bytes and describes "dst".
 * return:
 *	number of bits, either imputed classfully or specified with /CIDR,
 *	or -1 if some failure occurred (check errno).  ENOENT means it was
 *	not an IPv4 network specification.
 * note:
 *	network byte order assumed.  this means 192.5.5.240/28 has
 *	0b11110000 in its fourth octet.
 * author:
 *	Paul Vixie (ISC), June 1996
 */
static int
inet_cidr_pton_ipv4(const char *src, u_char *dst, size_t size)
{
    static const char xdigits[] = "0123456789abcdef";
    static const char digits[] = "0123456789";
    int			n,
                ch,
                tmp = 0,
                dirty,
                bits;
    const u_char *odst = dst;

    ch = *src++;
    if (ch == '0' && (src[0] == 'x' || src[0] == 'X')
        && isxdigit((unsigned char) src[1]))
    {
        /* Hexadecimal: Eat nybble string. */
        if (size <= 0U)
            goto emsgsize;
        dirty = 0;
        src++;					/* skip x or X. */
        while ((ch = *src++) != '\0' && isxdigit((unsigned char) ch))
        {
            if (isupper((unsigned char) ch))
                ch = tolower((unsigned char) ch);
            n = strchr(xdigits, ch) - xdigits;
            assert(n >= 0 && n <= 15);
            if (dirty == 0)
                tmp = n;
            else
                tmp = (tmp << 4) | n;
            if (++dirty == 2)
            {
                if (size-- <= 0U)
                    goto emsgsize;
                *dst++ = (u_char) tmp;
                dirty = 0;
            }
        }
        if (dirty)
        {						/* Odd trailing nybble? */
            if (size-- <= 0U)
                goto emsgsize;
            *dst++ = (u_char) (tmp << 4);
        }
    }
    else if (isdigit((unsigned char) ch))
    {
        /* Decimal: eat dotted digit string. */
        for (;;)
        {
            tmp = 0;
            do
            {
                n = strchr(digits, ch) - digits;
                assert(n >= 0 && n <= 9);
                tmp *= 10;
                tmp += n;
                if (tmp > 255)
                    goto enoent;
            } while ((ch = *src++) != '\0' &&
                     isdigit((unsigned char) ch));
            if (size-- <= 0U)
                goto emsgsize;
            *dst++ = (u_char) tmp;
            if (ch == '\0' || ch == '/')
                break;
            if (ch != '.')
                goto enoent;
            ch = *src++;
            if (!isdigit((unsigned char) ch))
                goto enoent;
        }
    }
    else
        goto enoent;

    bits = -1;
    if (ch == '/' && isdigit((unsigned char) src[0]) && dst > odst)
    {
        /* CIDR width specifier.  Nothing can follow it. */
        ch = *src++;			/* Skip over the /. */
        bits = 0;
        do
        {
            n = strchr(digits, ch) - digits;
            assert(n >= 0 && n <= 9);
            bits *= 10;
            bits += n;
        } while ((ch = *src++) != '\0' && isdigit((unsigned char) ch));
        if (ch != '\0')
            goto enoent;
        if (bits > 32)
            goto emsgsize;
    }

    /* Firey death and destruction unless we prefetched EOS. */
    if (ch != '\0')
        goto enoent;

    /* If nothing was written to the destination, we found no address. */
    if (dst == odst)
        goto enoent;
    /* If no CIDR spec was given, infer width from net class. */
    if (bits == -1)
    {
        if (*odst >= 240)		/* Class E */
            bits = 32;
        else if (*odst >= 224)	/* Class D */
            bits = 8;
        else if (*odst >= 192)	/* Class C */
            bits = 24;
        else if (*odst >= 128)	/* Class B */
            bits = 16;
        else
            /* Class A */
            bits = 8;
        /* If imputed mask is narrower than specified octets, widen. */
        if (bits < ((dst - odst) * 8))
            bits = (dst - odst) * 8;

        /*
         * If there are no additional bits specified for a class D address
         * adjust bits to 4.
         */
        if (bits == 8 && *odst == 224)
            bits = 4;
    }
    /* Extend network to cover the actual mask. */
    while (bits > ((dst - odst) * 8))
    {
        if (size-- <= 0U)
            goto emsgsize;
        *dst++ = '\0';
    }
    return bits;

enoent:
    errno = ENOENT;
    return -1;

emsgsize:
    errno = EMSGSIZE;
    return -1;
}

/*
 * int
 * inet_net_pton(af, src, dst, *bits)
 *	convert network address from presentation to network format.
 *	accepts inet_pton()'s input for this "af" plus trailing "/CIDR".
 *	"dst" is assumed large enough for its "af".  "bits" is set to the
 *	/CIDR prefix length, which can have defaults (like /32 for IPv4).
 * return:
 *	-1 if an error occurred (inspect errno; ENOENT means bad format).
 *	0 if successful conversion occurred.
 * note:
 *	192.5.5.1/28 has a nonzero host part, which means it isn't a network
 *	as called for by inet_cidr_pton() but it can be a host address with
 *	an included netmask.
 * author:
 *	Paul Vixie (ISC), October 1998
 */
static int
inet_net_pton_ipv4(const char *src, u_char *dst)
{
    static const char digits[] = "0123456789";
    const u_char *odst = dst;
    int			n,
                ch,
                tmp,
                bits;
    size_t		size = 4;

    /* Get the mantissa. */
    while (ch = *src++, isdigit((unsigned char) ch))
    {
        tmp = 0;
        do
        {
            n = strchr(digits, ch) - digits;
            assert(n >= 0 && n <= 9);
            tmp *= 10;
            tmp += n;
            if (tmp > 255)
                goto enoent;
        } while ((ch = *src++) != '\0' && isdigit((unsigned char) ch));
        if (size-- == 0)
            goto emsgsize;
        *dst++ = (u_char) tmp;
        if (ch == '\0' || ch == '/')
            break;
        if (ch != '.')
            goto enoent;
    }

    /* Get the prefix length if any. */
    bits = -1;
    if (ch == '/' && isdigit((unsigned char) src[0]) && dst > odst)
    {
        /* CIDR width specifier.  Nothing can follow it. */
        ch = *src++;			/* Skip over the /. */
        bits = 0;
        do
        {
            n = strchr(digits, ch) - digits;
            assert(n >= 0 && n <= 9);
            bits *= 10;
            bits += n;
        } while ((ch = *src++) != '\0' && isdigit((unsigned char) ch));
        if (ch != '\0')
            goto enoent;
        if (bits > 32)
            goto emsgsize;
    }

    /* Firey death and destruction unless we prefetched EOS. */
    if (ch != '\0')
        goto enoent;

    /* Prefix length can default to /32 only if all four octets spec'd. */
    if (bits == -1)
    {
        if (dst - odst == 4)
            bits = 32;
        else
            goto enoent;
    }

    /* If nothing was written to the destination, we found no address. */
    if (dst == odst)
        goto enoent;

    /* If prefix length overspecifies mantissa, life is bad. */
    if ((bits / 8) > (dst - odst))
        goto enoent;

    /* Extend address to four octets. */
    while (size-- > 0)
        *dst++ = 0;

    return bits;

enoent:
    errno = ENOENT;
    return -1;

emsgsize:
    errno = EMSGSIZE;
    return -1;
}

static int
getbits(const char *src, int *bitsp)
{
    static const char digits[] = "0123456789";
    int			n;
    int			val;
    char		ch;

    val = 0;
    n = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        pch = strchr(digits, ch);
        if (pch != NULL)
        {
            if (n++ != 0 && val == 0)	/* no leading zeros */
                return 0;
            val *= 10;
            val += (pch - digits);
            if (val > 128)		/* range */
                return 0;
            continue;
        }
        return 0;
    }
    if (n == 0)
        return 0;
    *bitsp = val;
    return 1;
}

static int
getv4(const char *src, u_char *dst, int *bitsp)
{
    static const char digits[] = "0123456789";
    u_char	   *odst = dst;
    int			n;
    u_int		val;
    char		ch;

    val = 0;
    n = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        pch = strchr(digits, ch);
        if (pch != NULL)
        {
            if (n++ != 0 && val == 0)	/* no leading zeros */
                return 0;
            val *= 10;
            val += (pch - digits);
            if (val > 255)		/* range */
                return 0;
            continue;
        }
        if (ch == '.' || ch == '/')
        {
            if (dst - odst > 3) /* too many octets? */
                return 0;
            *dst++ = val;
            if (ch == '/')
                return getbits(src, bitsp);
            val = 0;
            n = 0;
            continue;
        }
        return 0;
    }
    if (n == 0)
        return 0;
    if (dst - odst > 3)			/* too many octets? */
        return 0;
    *dst++ = val;
    return 1;
}

static int
inet_net_pton_ipv6(const char *src, u_char *dst)
{
    return inet_cidr_pton_ipv6(src, dst, 16);
}


static int
inet_cidr_pton_ipv6(const char *src, u_char *dst, size_t size)
{
    static const char xdigits_l[] = "0123456789abcdef",
                xdigits_u[] = "0123456789ABCDEF";
    u_char		tmp[NS_IN6ADDRSZ],
               *tp,
               *endp,
               *colonp;
    const char *xdigits,
               *curtok;
    int			ch,
                saw_xdigit;
    u_int		val;
    int			digits;
    int			bits;

    if (size < NS_IN6ADDRSZ)
        goto emsgsize;

    memset((tp = tmp), '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            goto enoent;
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    digits = 0;
    bits = -1;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (++digits > 4)
                goto enoent;
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                    goto enoent;
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
                goto enoent;
            if (tp + NS_INT16SZ > endp)
                goto enoent;
            *tp++ = (u_char) (val >> 8) & 0xff;
            *tp++ = (u_char) val & 0xff;
            saw_xdigit = 0;
            digits = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
            getv4(curtok, tp, &bits) > 0)
        {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break;				/* '\0' was seen by inet_pton4(). */
        }
        if (ch == '/' && getbits(src, &bits) > 0)
            break;
        goto enoent;
    }
    if (saw_xdigit)
    {
        if (tp + NS_INT16SZ > endp)
            goto enoent;
        *tp++ = (u_char) (val >> 8) & 0xff;
        *tp++ = (u_char) val & 0xff;
    }
    if (bits == -1)
        bits = 128;

    endp = tmp + 16;

    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle overlapping
         * regions, we'll do the shift by hand.
         */
        const int	n = tp - colonp;
        int			i;

        if (tp == endp)
            goto enoent;
        for (i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        goto enoent;

    /*
     * Copy out the result.
     */
    memcpy(dst, tmp, NS_IN6ADDRSZ);

    return bits;

enoent:
    errno = ENOENT;
    return -1;

emsgsize:
    errno = EMSGSIZE;
    return -1;
}

/*
 * Verify a CIDR address is OK (doesn't have bits set past the masklen)
 */
static bool address_ok(unsigned char *a, int bits, int family)
{
    int			byte;
    int			nbits;
    int			maxbits;
    int			maxbytes;
    unsigned char mask;

    if (family == PGSQL_AF_INET)
    {
        maxbits = 32;
        maxbytes = 4;
    }
    else
    {
        maxbits = 128;
        maxbytes = 16;
    }
    assert(bits <= maxbits);

    if (bits == maxbits)
        return true;

    byte = bits / 8;

    nbits = bits % 8;
    mask = 0xff;
    if (bits != 0)
        mask >>= nbits;

    while (byte < maxbytes)
    {
        if ((a[byte] & mask) != 0)
            return false;
        mask = 0xff;
        byte++;
    }

    return true;
}


} // ns: pq_async