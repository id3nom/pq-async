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

#include "pq-async/pg_type_date_def.h"
#include "pq-async/pg_type_duration_def.h"

namespace pq_async{ namespace tests{

class date_types_test
    : public db_test_base
{
public:
};

TEST_F(date_types_test, timestamp_array)
{
    try{
        auto a =
            db->query_value<pq_async::arr_timestamp>(
                "select ARRAY["
                "'2017-09-26 22:01:00.123456'::"
                "timestamp without time zone, "
                "'2017-09-27 22:01:00.123456'::"
                "timestamp without time zone"
                "]"
            );
        std::cout << "a[0]: " << a[0].iso_string() << std::endl;
        std::cout << "a[1]: " << a[1].iso_string() << std::endl;
        auto b = a;
        a = db->query_value<pq_async::arr_timestamp>(
            "select $1", b
        );
        std::cout << "a[0]: " << a[0].iso_string() << std::endl;
        std::cout << "a[1]: " << a[1].iso_string() << std::endl;
        std::cout << "b[0]: " << b[0].iso_string() << std::endl;
        std::cout << "b[1]: " << b[1].iso_string() << std::endl;
        ASSERT_THAT(a[0].iso_string(), testing::Eq(b[0].iso_string()));
        ASSERT_THAT(a[1].iso_string(), testing::Eq(b[1].iso_string()));
        
        auto r =
            db->query_single(
                "select ARRAY["
                "'2017-09-26 22:01:00.123456'::"
                "timestamp without time zone, "
                "'2017-09-27 22:01:00.123456'::"
                "timestamp without time zone"
                "] as arr"
            );
        
        arr_timestamp a1 = r->as< arr_timestamp >("arr");
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, timestamp_from_bin)
{
    try{
        auto ts0 =
            db->query_value<pq_async::timestamp>(
                "select '2017-09-26 22:01:00.123456'::"
                "timestamp without time zone"
            );
        auto ts0b = ts0;
        ts0 = db->query_value<pq_async::timestamp>(
            "select $1", ts0b
        );
        std::cout << "ts0 iso_string: " << ts0.iso_string() << std::endl;
        std::cout << "ts0b iso_string:" << ts0b.iso_string() << std::endl;
        ASSERT_THAT(ts0.iso_string(), testing::Eq(ts0b.iso_string()));
        
        auto ts1 =
            db->query_value<pq_async::timestamp>(
                "select '0001-01-01 09:08:07.123456 BC'::"
                "timestamp without time zone"
            );
        auto ts2 =
            db->query_value<pq_async::timestamp>(
                "select '0500-01-01 09:08:07.123456 BC'::"
                "timestamp without time zone"
            );
        
        std::cout << "ts0 iso_string:" << ts0.iso_string() << std::endl;
        std::cout << "ts1 iso_string:" << ts1.iso_string() << std::endl;
        std::cout << "ts2 iso_string:" << ts2.iso_string() << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("2017-09-26 22:01:00.123456")
        );
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("-0001-01-01 09:08:07.123456")
        );
        ASSERT_THAT(
            ts2.iso_string(), 
            testing::Eq("-0500-01-01 09:08:07.123456")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, timestamp_from_txt)
{
    try{
        auto txt0 =
            db->query_value<std::string>(
                "select '2017-09-26 22:01:00.123456'::"
                "timestamp without time zone::text"
            );
        
        auto ts0 = pq_async::timestamp::parse(txt0);
        std::cout << "ts0 iso_string:" << ts0.iso_string() << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("2017-09-26 22:01:00.123456")
        );
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, timestamp_tz_from_bin)
{
    try{
        auto ts0 =
            db->query_value<pq_async::timestamp_tz>(
                "select '2017-09-26 19:08:07.123456 EST'::"
                "timestamp with time zone"
            );
        auto ts0b = ts0;
        ts0 = db->query_value<pq_async::timestamp_tz>(
            "select $1", ts0b
        );
        std::cout << "ts0 iso_string: " << ts0.iso_string() << std::endl;
        std::cout << "ts0b iso_string:" << ts0b.iso_string() << std::endl;
        ASSERT_THAT(ts0.iso_string(), testing::Eq(ts0b.iso_string()));
            
        auto ts1 =
            db->query_value<pq_async::timestamp_tz>(
                "select '0001-01-01 19:08:07.123456 EST BC'::"
                "timestamp with time zone"
            );
        auto ts2 =
            db->query_value<pq_async::timestamp_tz>(
                "select '0500-01-01 19:08:07.123456 EST BC'::"
                "timestamp with time zone"
            );
        auto ts3 =
            db->query_value<pq_async::timestamp_tz>(
                "select '0001-12-31 19:08:07.123456 EST BC'::"
                "timestamp with time zone"
            );
        
        auto tz0 = ts0.make_zoned("EST");
        auto tz1 = ts1.make_zoned("EST");
        auto tz2 = ts2.make_zoned("EST");
        auto tz3 = ts3.make_zoned("EST");
        
        std::cout << "ts0 iso_string:" << ts0.iso_string() << std::endl;
        std::cout << "tz0 iso_string:" << tz0.iso_string() << std::endl;
        std::cout << std::endl;
        
        std::cout << "ts1 iso_string:" << ts1.iso_string() << std::endl;
        std::cout << "tz1 iso_string:" << tz1.iso_string() << std::endl;
        std::cout << std::endl;
        
        std::cout << "ts2 iso_string:" << ts2.iso_string() << std::endl;
        std::cout << "tz2 iso_string:" << tz2.iso_string() << std::endl;
        std::cout << std::endl;
        
        std::cout << "ts3 iso_string:" << ts3.iso_string() << std::endl;
        std::cout << "tz3 iso_string:" << tz3.iso_string() << std::endl;
        std::cout << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("2017-09-27 00:08:07.123456+0000")
        );
        ASSERT_THAT(
            tz0.iso_string(), 
            testing::Eq("2017-09-26 19:08:07.123456-0500")
        );
        
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("-0001-01-02 00:08:07.123456+0000")
        );
        ASSERT_THAT(
            tz1.iso_string(), 
            testing::Eq("-0001-01-01 19:08:07.123456-0500")
        );
        
        ASSERT_THAT(
            ts2.iso_string(), 
            testing::Eq("-0500-01-02 00:08:07.123456+0000")
        );
        ASSERT_THAT(
            tz2.iso_string(), 
            testing::Eq("-0500-01-01 19:08:07.123456-0500")
        );
        
        ASSERT_THAT(
            ts3.iso_string(), 
            testing::Eq("0001-01-01 00:08:07.123456+0000")
        );
        ASSERT_THAT(
            tz3.iso_string(), 
            testing::Eq("0000-12-31 19:08:07.123456-0500")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(date_types_test, timestamp_tz_make_zoned)
{
    try{
        auto ts0 =
            db->query_value<pq_async::timestamp_tz>(
                "select '2000-01-01 00:00:00.000000 UTC'::"
                "timestamp with time zone"
            );
        
        auto ts1 = ts0.make_zoned("EST");
        timestamp ts2(ts1);
        
        hhdate::zoned_time<std::chrono::microseconds> ts3 = ts1;
        
        std::cout
            << "ts0 iso_string: " << ts0.iso_string() << std::endl
            << "ts1 iso_string: " << ts1.iso_string() << std::endl
            << "ts2 iso_string: " << ts2.iso_string() << std::endl
            << "ts3 UTC: " <<
                hhdate::format("%F %T %Z", ts3.get_sys_time())
                << std::endl
            << "ts3 EST: " << ts3 << std::endl
            << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("2000-01-01 00:00:00.000000+0000")
        );
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("1999-12-31 19:00:00.000000-0500")
        );
        
        ASSERT_THAT(
            hhdate::format("%F %T %Z", ts3.get_sys_time()), 
            testing::Eq("2000-01-01 00:00:00.000000 UTC")
        );
        ASSERT_THAT(
            hhdate::format("%F %T %Z", ts3),
            testing::Eq("1999-12-31 19:00:00.000000 EST")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(date_types_test, timestamp_as_zone)
{
    try{
        auto ts0 =
            db->query_value<pq_async::timestamp>(
                "select '2000-01-01 00:00:00.000000'::"
                "timestamp without time zone"
            );
        
        auto ts1 = ts0.as_zone("EST");
        hhdate::zoned_time<std::chrono::microseconds> ts2 = ts1;
        
        std::cout
            << "ts0 iso_string: " << ts0.iso_string() << std::endl
            << "ts1 iso_string: " << ts1.iso_string() << std::endl
            << "ts2 UTC: " <<
                hhdate::format("%F %T %Z", ts2.get_sys_time())
                << std::endl
            << "ts2 EST: " << ts2 << std::endl
            << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("2000-01-01 00:00:00.000000")
        );
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("2000-01-01 00:00:00.000000-0500")
        );
        
        ASSERT_THAT(
            hhdate::format("%F %T %Z", ts2.get_sys_time()), 
            testing::Eq("2000-01-01 05:00:00.000000 UTC")
        );
        ASSERT_THAT(
            hhdate::format("%F %T %Z", ts2),
            testing::Eq("2000-01-01 00:00:00.000000 EST")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, interval_from_bin)
{
    /*
    int format = 1;
    int oid = interval;
    int len = 16;
    //select interval 'P0Y0M1DT0H0M0S'
    char val_a[] = {
        0, 0, 0, 0, 0, 0, 0, 0, //time
        0, 0, 0, 1, 0, 0, 0, 0  //day, month
    };
    //select interval 'P1Y2M3DT4H5M6S'
    char val_b[] = {
        0, 0, 0, 3, 108, -117, -64, -128, 
        0, 0, 0, 3, 0, 0, 0, 14
    };
    //select interval 'P7Y8M9DT1H2M3S'
    char val_c[] = {
        0, 0, 0, 0, -35, -24, 120, -64, 
        0, 0, 0, 9, 0, 0, 0, 92
    };
    */
    
    try{
        auto i0 =
            db->query_value<pq_async::interval>(
                "select interval 'P0Y0M1DT0H0M0S'"
            );
        std::cout << "i0 iso_string: " << i0.iso_string() << std::endl;
        auto i0b = i0;
        i0 = db->query_value<pq_async::interval>(
            "select $1", i0b
        );
        std::cout << "i0 iso_string: " << i0.iso_string() << std::endl;
        std::cout << "i0b iso_string:" << i0b.iso_string() << std::endl;
        ASSERT_THAT(i0.iso_string(), testing::Eq(i0b.iso_string()));
            
        auto i1 =
            db->query_value<pq_async::interval>(
                "select interval 'P1Y2M3DT4H5M6S'"
            );
        auto i2 =
            db->query_value<pq_async::interval>(
                "select interval 'P7Y8M9DT1H2M3.654S'"
            );
        auto i3 =
            db->query_value<pq_async::interval>(
                "select interval 'P0Y0M-1DT0H0M0S'"
            );
        
        std::cout
            << "i0 iso_string: " << i0.iso_string() << std::endl
            << "i1 iso_string: " << i1.iso_string() << std::endl
            << "i2 iso_string: " << i2.iso_string() << std::endl
            << "i3 iso_string: " << i3.iso_string() << std::endl
            << std::endl;
        
        ASSERT_THAT(i0.iso_string(), testing::Eq("P0Y0M1DT0H0M0S"));
        ASSERT_THAT(i1.iso_string(), testing::Eq("P1Y2M3DT4H5M6S"));
        ASSERT_THAT(i2.iso_string(), testing::Eq("P7Y8M9DT1H2M3.654000S"));
        ASSERT_THAT(i3.iso_string(), testing::Eq("P0Y0M-1DT0H0M0S"));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }	
}

TEST_F(date_types_test, date_from_bin)
{
    try{
        auto i0 =
            db->query_value<pq_async::date>(
                "select date '2018-10-05'"
            );
        auto i0b = i0;
        i0 = db->query_value<pq_async::date>(
            "select $1", i0b
        );
        std::cout << "i0 iso_string: " << i0.iso_string() << std::endl;
        std::cout << "i0b iso_string:" << i0b.iso_string() << std::endl;
        ASSERT_THAT(i0.iso_string(), testing::Eq(i0b.iso_string()));
            
        auto i1 =
            db->query_value<pq_async::date>(
                "select date '0001-01-01'"
            );
        auto i2 =
            db->query_value<pq_async::date>(
                "select date '0001-01-01 BC'"
            );
        auto i3 =
            db->query_value<pq_async::date>(
                "select date '0500-10-05 BC'"
            );
        
        std::cout
            << "i0 iso_string: " << i0.iso_string() << std::endl
            << "i1 iso_string: " << i1.iso_string() << std::endl
            << "i2 iso_string: " << i2.iso_string() << std::endl
            << "i3 iso_string: " << i3.iso_string() << std::endl
            << std::endl;
        
        ASSERT_THAT(i0.iso_string(), testing::Eq("2018-10-05"));
        ASSERT_THAT(i1.iso_string(), testing::Eq("0001-01-01"));
        ASSERT_THAT(i2.iso_string(), testing::Eq("-0001-01-01"));
        ASSERT_THAT(i3.iso_string(), testing::Eq("-0500-10-05"));
        
        
        hhdate::year_month_day ymd = hhdate::year(2018)/10/5;
        auto j0 = db->query_value<pq_async::date>(
            "select $1", (pq_async::date)ymd
        );
        ymd = hhdate::year(1)/1/1;
        auto j1 = db->query_value<pq_async::date>(
            "select $1", (pq_async::date)ymd
        );
        ymd = hhdate::year(-1)/1/1;
        auto j2 = db->query_value<pq_async::date>(
            "select $1", (pq_async::date)ymd
        );
        ymd = hhdate::year(-500)/10/5;
        auto j3 = db->query_value<pq_async::date>(
            "select $1", (pq_async::date)ymd
        );
        
        std::cout
            << "j0 iso_string: " << j0.iso_string() << std::endl
            << "j1 iso_string: " << j1.iso_string() << std::endl
            << "j2 iso_string: " << j2.iso_string() << std::endl
            << "j3 iso_string: " << j3.iso_string() << std::endl
            << std::endl;
        
        ASSERT_THAT(j0.iso_string(), testing::Eq("2018-10-05"));
        ASSERT_THAT(j1.iso_string(), testing::Eq("0001-01-01"));
        ASSERT_THAT(j2.iso_string(), testing::Eq("-0001-01-01"));
        ASSERT_THAT(j3.iso_string(), testing::Eq("-0500-10-05"));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(date_types_test, time_from_bin)
{
    try{
        auto ts0 =
            db->query_value<pq_async::time>(
                "select '22:01:00.123456'::"
                "time without time zone"
            );
        auto ts0b = ts0;
        ts0 = db->query_value<pq_async::time>(
            "select $1", ts0b
        );
        std::cout << "ts0 iso_string: " << ts0.iso_string() << std::endl;
        std::cout << "ts0b iso_string:" << ts0b.iso_string() << std::endl;
        ASSERT_THAT(ts0.iso_string(), testing::Eq(ts0b.iso_string()));
            
        auto tsmin =
            db->query_value<pq_async::time>(
                "select '00:00:00.000000'::"
                "time without time zone"
            );
        auto tsmax =
            db->query_value<pq_async::time>(
                "select '23:59:59.999999'::"
                "time without time zone"
            );
        
        std::cout << "ts0 iso_string:" << ts0.iso_string() << std::endl;
        std::cout << "tsmin iso_string:" << tsmin.iso_string() << std::endl;
        std::cout << "tsmax iso_string:" << tsmax.iso_string() << std::endl;
        
        ASSERT_THAT(ts0.iso_string(), testing::Eq("22:01:00.123456"));
        ASSERT_THAT(tsmin.iso_string(), testing::Eq("00:00:00.000000"));
        ASSERT_THAT(tsmax.iso_string(), testing::Eq("23:59:59.999999"));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, time_zone_info)
{
    try{
        auto z = hhdate::current_zone();
        
        // winter info
        hhdate::sys_info wi = z->get_info(
            hhdate::sys_days{hhdate::year(2018)/01/01} + 
            std::chrono::hours(0) + std::chrono::minutes(0)
        );
        // summer info
        hhdate::sys_info si = z->get_info(
            hhdate::sys_days{hhdate::year(2018)/06/01} + 
            std::chrono::hours(0) + std::chrono::minutes(0)
        );
        
        std::cout << "current zone name: " << z->name()
            << "\n--------------------------------" << std::endl;
        
        std::cout << "winter info" << 
            ", abbrev: " << wi.abbrev <<
            ", offset: " << 
                pq_async::duration::from_seconds(wi.offset.count()) <<
            ", begin: " << 
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        wi.begin.time_since_epoch()
                    )
                ) <<
            ", end: " <<
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        wi.end.time_since_epoch()
                    )
                ) <<
            std::endl;
        
        std::cout << "summer info" << 
            ", abbrev: " << si.abbrev <<
            ", offset: " <<
                pq_async::duration::from_seconds(si.offset.count()) <<
            ", begin: " << 
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        si.begin.time_since_epoch()
                    )
                ) <<
            ", end: " <<
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        si.end.time_since_epoch()
                    )
                ) <<
            std::endl;
            
        std::cout << std::endl;
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(date_types_test, time_tz_from_bin)
{
    try{
        auto ts0 =
            db->query_value<pq_async::time_tz>(
                "select '00:00:00.000000 UTC'::"
                "time with time zone"
            );
        auto ts0b = ts0;
        ts0 = db->query_value<pq_async::time_tz>(
            "select $1", ts0b
        );
        std::cout << "ts0 iso_string: " << ts0.iso_string() << std::endl;
        std::cout << "ts0b iso_string:" << ts0b.iso_string() << std::endl;
        ASSERT_THAT(ts0.iso_string(), testing::Eq(ts0b.iso_string()));
        
        auto tsmin =
            db->query_value<pq_async::time_tz>(
                "select '00:00:00.000000 AEST'::"
                "time with time zone"
            );
        auto tsmax =
            db->query_value<pq_async::time_tz>(
                "select '00:00:00.000000 HST'::"
                "time with time zone"
            );
        
        auto z = hhdate::current_zone();
        auto zi = z->get_info(
            (hhdate::sys_time<std::chrono::minutes>)(timestamp_tz())
        );
        std::cout << "current zone name: " << z->name()
            << "\n--------------------------------" << std::endl;
        std::cout << "zone info" << 
            ", abbrev: " << zi.abbrev <<
            ", offset: " << 
                pq_async::duration::from_seconds(zi.offset.count()) <<
            ", begin: " << 
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        zi.begin.time_since_epoch()
                    )
                ) <<
            ", end: " <<
                hhdate::format(
                    "%Y-%m-%d %T", 
                    hhdate::sys_time<std::chrono::microseconds>(
                        zi.end.time_since_epoch()
                    )
                ) <<
            std::endl;
        
        int32_t zi_h = zi.offset.count() / 3600;
        int32_t zi_m = (zi.offset.count() % 3600) / 60;
        int32_t zi_m_asb =
            zi_h < 0 ?
                (zi_m == 0 ? 0 : 60 - std::abs(zi_m)) :
                zi_m;
        
        std::stringstream ss;
        if(zi_h < 0){
            ss << 
                std::setfill('0') << std::setw(2) << (24 + zi_h) <<
                ":" <<
                std::setfill('0') << std::setw(2) << zi_m_asb <<
                ":00.000000-" <<
                std::setfill('0') << std::setw(2) << std::abs(zi_h) <<
                std::setfill('0') << std::setw(2) << std::abs(zi_m);
            
        } else {
            ss <<
                std::setfill('0') << std::setw(2) << (0 + zi_h) <<
                ":" <<
                std::setfill('0') << std::setw(2) << zi_m_asb <<
                ":00.000000+" <<
                std::setfill('0') << std::setw(2) << std::abs(zi_h) <<
                std::setfill('0') << std::setw(2) << std::abs(zi_m);
        }
        
        std::string ts0f = ss.str();
        
        //ts0 = ts0.make_zoned(z->name().c_str());
        
        ts0 = ts0.make_zoned(z);
        hhdate::zoned_time<std::chrono::microseconds> tzc = ts0;
        std::cout << tzc << std::endl;
        
        auto tsmin1 = tsmin.make_zoned("Australia/Melbourne");
        //auto tsmin1 = tsmin.make_zoned("Pacific/Auckland");
        auto tsmax1 = tsmin.make_zoned("HST");
        
        std::cout << "ts0 iso_string:" << ts0.iso_string() << std::endl;
        std::cout << "ts0f:          " << ts0f << std::endl;

        std::cout << "tsmin iso_string:" << tsmin.iso_string() << std::endl;
        std::cout << "tsmin-AEST iso_string:" << 
            tsmin1.iso_string() << std::endl;
        std::cout << "tsmax iso_string:" << tsmax.iso_string() << std::endl;
        std::cout << "tsmax-HST iso_string:" << 
            tsmax1.iso_string() << std::endl;
        
        //19:00:00.000000-0500 -18000
        //ASSERT_THAT(ts0.iso_string(), testing::Eq("22:01:00.123456"));
        ASSERT_THAT(ts0.iso_string(), testing::Eq(ts0f));
        ASSERT_THAT(tsmin.iso_string(), testing::Eq("00:00:00.000000+0000"));
        ASSERT_THAT(tsmin1.iso_string(), testing::Eq("11:00:00.000000+1100"));
        ASSERT_THAT(tsmax.iso_string(), testing::Eq("00:00:00.000000+0000"));
        ASSERT_THAT(tsmax1.iso_string(), testing::Eq("14:00:00.000000-1000"));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(date_types_test, time_tz_make_zoned)
{
    try{
        auto ts0 =
            db->query_value<pq_async::time_tz>(
                "select '00:00:00.000000 UTC'::"
                "time with time zone"
            );
        
        auto ts1 = ts0.make_zoned("EST");
        pq_async::time ts2(ts1);
        
        hhdate::zoned_time<std::chrono::microseconds> ts3 = ts1;
        
        std::cout
            << "ts0 iso_string: " << ts0.iso_string() << std::endl
            << "ts1 iso_string: " << ts1.iso_string() << std::endl
            << "ts2 iso_string: " << ts2.iso_string() << std::endl
            << "ts3 UTC: " <<
                hhdate::format("%T %Z", ts3.get_sys_time())
                << std::endl
            << "ts3 EST: " <<
                hhdate::format("%T %Z", ts3)
                << std::endl
            << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("00:00:00.000000+0000")
        );
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("19:00:00.000000-0500")
        );
        
        ASSERT_THAT(
            hhdate::format("%T %Z", ts3.get_sys_time()), 
            testing::Eq("00:00:00.000000 UTC")
        );
        ASSERT_THAT(
            hhdate::format("%T %Z", ts3),
            testing::Eq("19:00:00.000000 EST")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(date_types_test, time_as_zone)
{
    try{
        auto ts0 =
            db->query_value<pq_async::time>(
                "select '00:00:00.000000'::"
                "time without time zone"
            );
        
        auto ts1 = ts0.as_zone("EST");
        hhdate::zoned_time<std::chrono::microseconds> ts2 = ts1;
        
        std::cout
            << "ts0 iso_string: " << ts0.iso_string() << std::endl
            << "ts1 iso_string: " << ts1.iso_string() << std::endl
            << "ts2 UTC: " <<
                hhdate::format("%T %Z", ts2.get_sys_time())
                << std::endl
            << "ts2 EST: " <<
                hhdate::format("%T %Z", ts2)
                << std::endl
            << std::endl;
        
        ASSERT_THAT(
            ts0.iso_string(), 
            testing::Eq("00:00:00.000000")
        );
        ASSERT_THAT(
            ts1.iso_string(), 
            testing::Eq("00:00:00.000000-0500")
        );
        
        ASSERT_THAT(
            hhdate::format("%T %Z", ts2.get_sys_time()), 
            testing::Eq("05:00:00.000000 UTC")
        );
        ASSERT_THAT(
            hhdate::format("%T %Z", ts2),
            testing::Eq("00:00:00.000000 EST")
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}



}} //ns pq_async::tests
