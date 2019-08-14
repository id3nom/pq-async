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

class cash_types_test
    : public db_test_base
{
public:
};


#include <iostream>       // std::cout
#include <locale>         // std::locale, std::moneypunct, std::use_facet

// overload inserter to print patterns:
std::ostream& operator<< (std::ostream& os, std::moneypunct<char>::pattern p)
{
    for (int i=0; i<4; i++)
        switch (p.field[i]) {
            case std::moneypunct<char>::none: std::cout << "none "; break;
            case std::moneypunct<char>::space: std::cout << "space "; break;
            case std::moneypunct<char>::symbol: std::cout << "symbol "; break;
            case std::moneypunct<char>::sign: std::cout << "sign "; break;
            case std::moneypunct<char>::value: std::cout << "value "; break;
        }
    return os;
}

TEST_F(cash_types_test, cash_test_bin)
{
    try{
        std::string pg_locale = db->query_value<std::string>(
            "select setting from pg_settings where name = 'lc_monetary'"
        );
        //std::setlocale(LC_MONETARY, pg_locale.c_str());
        //std::locale::global(std::locale(pg_locale.c_str()));
        
        std::locale mylocale("");
        const std::moneypunct<char>& mp = 
            std::use_facet<std::moneypunct<char> >(mylocale);

        std::cout << "moneypunct in locale \"" << mylocale.name() << "\":\n";

        std::cout << "decimal_point: " << mp.decimal_point() << '\n';
        std::cout << "thousands_sep: " << mp.thousands_sep() << '\n';
        std::cout << "grouping: " << mp.grouping() << '\n';
        std::cout << "curr_symbol: " << mp.curr_symbol() << '\n';
        std::cout << "positive_sign: " << mp.positive_sign() << '\n';
        std::cout << "negative_sign: " << mp.negative_sign() << '\n';
        std::cout << "frac_digits: " << mp.frac_digits() << '\n';
        std::cout << "pos_format: " << mp.pos_format() << '\n';
        std::cout << "neg_format: " << mp.neg_format() << '\n';
        
        
        pq_async::money::set_locale(std::locale("en_US.UTF-8"));
        
        auto m = db->query_value<money>(
            "select '12.543'::money "
        );
        
        pq_async::numeric n = m;// m.to_numeric(2);
        
        std::cout << "m: " << m << std::endl;
        std::cout << "n: " << n << std::endl;
        
        ASSERT_THAT((std::string)m, testing::Eq("12.54"));
        ASSERT_THAT((std::string)n, testing::Eq("12.54"));
        
        ++m;
        ASSERT_THAT((std::string)m, testing::Eq("13.54"));
        
        m += 1.22;
        ASSERT_THAT((std::string)m, testing::Eq("14.76"));
        
        double d = m;
        ASSERT_THAT(d, testing::Eq(14.76));
        
        m = 12.53;
        std::cout << "m: " << m << std::endl;
        
        m = 100;
        m /= 2;
        std::cout << "m: " << m << std::endl;
        ASSERT_THAT((std::string)m, testing::Eq("50.00"));
        
        m *= 3;
        std::cout << "m: " << m << std::endl;
        ASSERT_THAT((std::string)m, testing::Eq("150.00"));
        
        m = pq_async::money::from_numeric("92233720368547758.07");
        // m = db->query_value<money>(
        // 	"select '92233720368547758.07'::money "
        // );
        m = db->query_value<money>(
            "select $1 ", m
        );
        
        std::cout << "m: " << m << std::endl;
        ASSERT_THAT(m.to_string(2), testing::Eq("92233720368547758.07"));
        n = m.to_numeric(2);
        std::cout << "n: " << n << std::endl;

        ASSERT_THAT(m.to_string(), testing::Eq((std::string)n));
        
        m = db->query_value<money>(
            "select '-92233720368547758.08'::money "
        );
        ASSERT_THAT(m.to_string(2), testing::Eq("-92233720368547758.08"));
        n = m.to_numeric(2);
        std::cout << "m: " << m.to_string(2) << std::endl;
        std::cout << "n: " << n << std::endl;
        ASSERT_THAT(m.to_string(2), testing::Eq((std::string)n));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}




}} //namespace pq_async::tests