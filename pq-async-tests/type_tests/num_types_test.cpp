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

class num_types_test
    : public db_test_base
{
public:
};


TEST_F(num_types_test, numeric_test_bin)
{
    try{
        numeric a("12.54");
        ASSERT_THAT((std::string)a, testing::Eq("12.54"));
        
        auto n = db->query_value<numeric>(
            "select '12.54'::numeric "
        );

        n = db->query_value<numeric>(
            "select $1::numeric ", n
        );
        
        if(a != n)
            FAIL();
        
        ASSERT_THAT((std::string)++a, testing::Eq("13.54"));
        ASSERT_THAT((std::string)a++, testing::Eq("13.54"));
        ASSERT_THAT((std::string)a, testing::Eq("14.54"));
        
        a += 2.2;
        a += 34;
        
        //int32_t b = a;
        
        n = db->query_value<numeric>(
            "select $1::numeric ", a
        );
        
        std::cout << "a:" << a << ", n:" << n << std::endl;
        
        if(a != n)
            FAIL();		
        
        std::cout << a << std::endl;
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}




}} //namespace pq_async::tests