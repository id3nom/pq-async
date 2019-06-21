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

class nullable_test
    : public db_test_base
{
public:
    void drop_table()
    {
    }
    
    void create_table()
    {
        this->drop_table();
    }
    
    void SetUp() override
    {
        db_test_base::SetUp();
        this->create_table();
    }
    
    void TearDown() override
    {
        this->drop_table();
        db_test_base::TearDown();
    }
    
    template<typename... Params>
    void exec_null_test(
        bool drop_table,
        std::string type_name,
        std::vector<std::string> flds,
        const Params&... args)
    {
        if(drop_table)
            db->execute(
                ("drop table if exists null_" + type_name + "_test").c_str()
            );
        
        std::string flds_def;
        for(size_t i = 0; i < flds.size(); ++i){
            flds_def +=  flds[i] + " NULL,";
            flds[i] = flds[i].substr(0, flds[i].find(" "));
        }
        if(drop_table)
            db->execute(
                ("create table null_" + type_name + "_test("
                    "id serial NOT NULL," +
                    flds_def +
                    "CONSTRAINT null_" + type_name +
                    "_test_pkey PRIMARY KEY (id)"
                ");").c_str()
            );
        
        std::string r(fmt::format(
            "insert into null_{}_test (",
            type_name
        ));
        std::string ps;
        for(size_t i = 0; i < flds.size(); ++i){
            r += flds[i];
            ps += "$" + md::num_to_str(i+1, false);
            if(i < flds.size() -1){
                r += ",";
                ps += ",";
            }
        }
        
        r += ") values (" + ps + ")";
        db->execute(r.c_str(), args...);
    }
    
};


TEST_F(nullable_test, nullable_bin_types)
{
    try{
        exec_null_test(
            true,
            "bin",
            {"f_a bytea"}, 
            pq_async::null<std::vector<int8_t>>()
        );
        exec_null_test(
            false,
            "bin",
            {"f_a bytea"}, 
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_bool_types)
{
    try{
        exec_null_test(
            true,
            "bool",
            {"f_a bool"}, 
            pq_async::null<bool>()
        );
        exec_null_test(
            false,
            "bool",
            {"f_a bool"}, 
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(nullable_test, nullable_chr_types)
{
    try{
        exec_null_test(
            true,
            "chr",
            {
                "f_a varchar(255)",
                "f_b char(5)",
                "f_c text"
            },
            pq_async::null<std::string>(),
            pq_async::null<std::string>(),
            pq_async::null<std::string>()
        );
        exec_null_test(
            false,
            "chr",
            {
                "f_a varchar(255)",
                "f_b char(5)",
                "f_c text"
            },
            nullptr,
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_datetime_types)
{
    try{
        exec_null_test(
            true,
            "datetime",
            {
                "f_a timestamp without time zone",
                "f_b timestamp with time zone",
                "f_c date",
                "f_d time without time zone",
                "f_e time with time zone",
                "f_f interval"
            },
            pq_async::null<pq_async::timestamp>(),
            pq_async::null<pq_async::timestamp_tz>(),
            pq_async::null<pq_async::date>(),
            pq_async::null<pq_async::time>(),
            pq_async::null<pq_async::time_tz>(),
            pq_async::null<pq_async::interval>()
        );
        exec_null_test(
            false,
            "datetime",
            {
                "f_a timestamp without time zone",
                "f_b timestamp with time zone",
                "f_c date",
                "f_d time without time zone",
                "f_e time with time zone",
                "f_f interval"
            },
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
        auto eq = md::event_queue_t::get_default();
        
        db->exec_queries(
            "DO $BODY$ "
            "BEGIN "
            "IF NOT EXISTS (SELECT 1 FROM pg_type "
            "WHERE typname = 'narrstates') THEN "
            "CREATE TYPE narrstates as ENUM "
            "('new', 'in progress', 'completed');"
            "END IF;"
            "END $BODY$;"
        );
        
        db->execute(
            "drop table if exists null_array_test"
        );
        db->execute(
            "CREATE TABLE null_array_test("
            "    id serial NOT NULL,"
                
            "    title text NOT NULL,"
                
            "    lineups int[] NOT NULL,"
            "    channels int[] NOT NULL,"
                
            "    state narrstates NOT NULL,"
                
            "    start_date timestamp with time zone NULL,"
            "    end_date timestamp with time zone NULL,"
                
            "    created timestamp NOT NULL DEFAULT now(),"
            "    modified timestamp NOT NULL DEFAULT now(),"
                
            "    CONSTRAINT null_array_test_pkey PRIMARY KEY (id)"
            ")WITH(OIDS=false);"
        );
        
        std::string title("test");
        auto start_date = pq_async::timestamp_tz::null();
        auto end_date = pq_async::timestamp_tz::null();
        
        db->execute(
            "insert into null_array_test "
            "(title, lineups, channels, state, start_date, end_date) values "
            "($1, '{0,1,2}', '{3,4,5}', 'new', $2, $3) returning id",
            title, start_date, end_date,
            [](auto err, int /*nb*/){
                if(err){
                    throw MD_ERR(err.c_str());
                }
            }
        );
        
        db->query_value<int>(
            "insert into null_array_test "
            "(title, lineups, channels, state, start_date, end_date) values "
            "($1, '{0,1,2}', '{3,4,5}', 'new', $2, $3) returning id",
            title, start_date, end_date,
            [](auto err, int new_id){
                if(err){
                    throw MD_ERR(err.c_str());
                }
                std::cout << new_id << std::endl;
            }
        );
        
        eq->run();
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_geo_types)
{
    try{
        exec_null_test(
            true,
            "geo",
            {
                "f_a point",
                "f_b line",
                "f_c lseg",
                "f_d box",
                "f_e path",
                "f_f polygon",
                "f_g circle",
            },
            pq_async::null<pq_async::point>(),
            pq_async::null<pq_async::line>(),
            pq_async::null<pq_async::lseg>(),
            pq_async::null<pq_async::box>(),
            pq_async::null<pq_async::path>(),
            pq_async::null<pq_async::polygon>(),
            pq_async::null<pq_async::circle>()
        );
        exec_null_test(
            false,
            "geo",
            {
                "f_a point",
                "f_b line",
                "f_c lseg",
                "f_d box",
                "f_e path",
                "f_f polygon",
                "f_g circle",
            },
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_json_types)
{
    try{
        exec_null_test(
            true,
            "json",
            {
                "f_a json",
                "f_b jsonb",
            },
            pq_async::null<pq_async::json>(),
            pq_async::null<pq_async::json>()
        );
        exec_null_test(
            false,
            "json",
            {
                "f_a json",
                "f_b jsonb",
            },
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_cash_types)
{
    try{
        exec_null_test(
            true,
            "cash",
            {
                "f_a money",
            },
            pq_async::null<pq_async::money>()
        );
        exec_null_test(
            false,
            "cash",
            {
                "f_a money",
            },
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_net_types)
{
    try{
        exec_null_test(
            true,
            "net",
            {
                "f_a cidr",
                "f_b inet",
                "f_c macaddr",
                "f_d macaddr8",
            },
            pq_async::null<pq_async::cidr>(),
            pq_async::null<pq_async::inet>(),
            pq_async::null<pq_async::macaddr>(),
            pq_async::null<pq_async::macaddr8>()
        );
        exec_null_test(
            false,
            "net",
            {
                "f_a cidr",
                "f_b inet",
                "f_c macaddr",
                "f_d macaddr8",
            },
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_num_types)
{
    try{
        exec_null_test(
            true,
            "num",
            {
                "f_a smallint",
                "f_b integer",
                "f_c bigint",
                "f_d numeric",
                "f_e real",
                "f_f double precision",
            },
            pq_async::null<int16_t>(),
            pq_async::null<int32_t>(),
            pq_async::null<int64_t>(),
            pq_async::null<pq_async::numeric>(),
            pq_async::null<float>(),
            pq_async::null<double>()
        );
        exec_null_test(
            false,
            "num",
            {
                "f_a smallint",
                "f_b integer",
                "f_c bigint",
                "f_d numeric",
                "f_e real",
                "f_f double precision",
            },
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_rng_types)
{
    try{
        exec_null_test(
            true,
            "rng",
            {
                "f_a int4range",
                "f_b int8range",
                "f_c numrange",
                "f_d tsrange",
                "f_e tstzrange",
                "f_f daterange",
            },
            pq_async::null<pq_async::int4range>(),
            pq_async::null<pq_async::int8range>(),
            pq_async::null<pq_async::numrange>(),
            pq_async::null<pq_async::tsrange>(),
            pq_async::null<pq_async::tstzrange>(),
            pq_async::null<pq_async::daterange>()
        );
        exec_null_test(
            false,
            "rng",
            {
                "f_a int4range",
                "f_b int8range",
                "f_c numrange",
                "f_d tsrange",
                "f_e tstzrange",
                "f_f daterange",
            },
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_sys_types)
{
    try{
        exec_null_test(
            true,
            "sys",
            {
                "f_a oid",
            },
            pq_async::null<pq_async::oid>()
        );
        exec_null_test(
            false,
            "sys",
            {
                "f_a oid",
            },
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(nullable_test, nullable_uuid_types)
{
    try{
        exec_null_test(
            true,
            "uuid",
            {
                "f_a uuid",
            },
            pq_async::null<pq_async::uuid>()
        );
        exec_null_test(
            false,
            "uuid",
            {
                "f_a uuid",
            },
            nullptr
        );
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //namespace pq_async::tests