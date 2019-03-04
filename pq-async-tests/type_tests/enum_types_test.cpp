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

#include <gmock/gmock.h>
#include "../db_test_base.h"

namespace pq_async{ namespace tests{

class enum_types_test
    : public db_test_base
{
public:
};


/*
//
create type mood as enum ('sad', 'ok', 'happy');
//
 select * from pg_enum;
 enumtypid | enumsortorder | enumlabel 
-----------+---------------+-----------
     22630 |             1 | sad
     22630 |             2 | ok
     22630 |             3 | happy


libpq_async_tests=> select n.nspname, t.typname, t.oid 
    from pg_type t 
    inner join pg_catalog.pg_namespace n ON n.oid = t.typnamespace
    where typtype = 'e';
 nspname | typname |  oid  
---------+---------+-------
 public  | mood    | 22630
(1 row)

*/


TEST_F(enum_types_test, enum_test_bin)
{
    std::cout << "Not Implemented!" << std::endl;
    //FAIL();
}




}} //namespace pq_async::tests