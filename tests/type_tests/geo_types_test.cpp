/*
    This file is part of libpq-async++
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

#include <gmock/gmock.h>
#include "../db_test_base.h"

#include "src/pg_type_date_def.h"

namespace pq_async{ namespace tests{

class geo_types_test
    : public db_test_base
{
public:
};


TEST_F(geo_types_test, geo_point_test_bin)
{
    try{
        pq_async::point a(1, 2);
        std::cout << "a: " << a.x() << ", y: " << a.y() << std::endl;
        a.x() = 45;
        std::cout << "a: " << a.x() << ", y: " << a.y() << std::endl;
        
        point b(3.2, 5.7);
        
        a = db->query_value<point>(
            "select point(3.2, 5.7)"
        );
        ASSERT_THAT(a, testing::Eq(b));
        
        a = db->query_value<point>(
            "select $1", b
        );
        ASSERT_THAT(a, testing::Eq(b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_line_test_bin)
{
    try{
        line a(-2, -1, -3);
        
        auto pa = a.point_from_x(4.0);
        auto pb = a.point_from_x(8.0);
        ASSERT_THAT(pa, testing::Eq(point(4, -11)));
        ASSERT_THAT(pb, testing::Eq(point(8, -19)));

        a = line(point(4, -11), point(8, -19));
        // should give Ax: -2, By: -1, C: -3
        pa = a.point_from_y(-11.0);
        pb = a.point_from_y(-19.0);
        ASSERT_THAT(pa, testing::Eq(point(4, -11)));
        ASSERT_THAT(pb, testing::Eq(point(8, -19)));
        
        a = line(point(-3, 4), point(3, 25));
        // should give Ax: 3.5, By: -1, C: 14.5
        ASSERT_THAT((double)a.a(), testing::Eq(3.5));
        ASSERT_THAT((double)a.b(), testing::Eq(-1));
        ASSERT_THAT((double)a.c(), testing::Eq(14.5));

        pa = a.point_from_x(-3);
        pb = a.point_from_x(3);
        ASSERT_THAT(pa, testing::Eq(point(-3, 4)));
        ASSERT_THAT(pb, testing::Eq(point(3, 25)));

        
        a = line(point(-3, -25), point(3, -4));
        // should give {3.5,-1,-14.5}
        ASSERT_THAT((double)a.a(), testing::Eq(3.5));
        ASSERT_THAT((double)a.b(), testing::Eq(-1));
        ASSERT_THAT((double)a.c(), testing::Eq(-14.5));
        
        pa = a.point_from_x(-3);
        pb = a.point_from_x(3);
        ASSERT_THAT(pa, testing::Eq(point(-3, -25)));
        ASSERT_THAT(pb, testing::Eq(point(3, -4)));
        
        
        a = line(point(4, 5), point(3, 5));
        // should give {0,-1,5}
        ASSERT_THAT((double)a.a(), testing::Eq(0));
        ASSERT_THAT((double)a.b(), testing::Eq(-1));
        ASSERT_THAT((double)a.c(), testing::Eq(5));
        
        pa = a.point_from_x(4);
        pb = a.point_from_x(5);
        ASSERT_THAT(pa, testing::Eq(point(4, 5)));
        ASSERT_THAT(pb, testing::Eq(point(5, 5)));
        
        a = db->query_value<line>(
            "select line(point(4, -11), point(8, -19))"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<line>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


TEST_F(geo_types_test, geo_lseg_test_bin)
{
    try{
        auto a = db->query_value<lseg>(
            "select lseg(point(4, -11), point(8, -19))"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<lseg>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_box_test_bin)
{
    try{
        auto a = db->query_value<box>(
            "select box(point(4, -11), point(8, -19))"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<box>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_path_open_test_bin)
{
    try{
        path c(
            "[(324.75,-674.25),(941.75,-326.55),"
            "(-359.17,89.52),(206.87,230.11)]"
        );
        
        std::cout << "c: " << (std::string)c << std::endl;
        
        auto a = db->query_value<path>(
            "select path '[(324.75,-674.25),(941.75,-326.55),"
            "(-359.17,89.52),(206.87,230.11)]'"
        );
        
        path b;
        b.closed() = false;
        b.points().push_back(point(324.75,-674.25));
        b.points().push_back(point(941.75,-326.55));
        b.points().push_back(point(-359.17,89.52));
        b.points().push_back(point(206.87,230.11));
        
        std::cout << "a: " << a << std::endl;
        a = db->query_value<path>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_path_closed_test_bin)
{
    try{
        auto a = db->query_value<path>(
            "select path '((324.75,-674.25),(941.75,-326.55),"
            "(-359.17,89.52),(206.87,230.11))'"
        );
        
        path b;
        b.closed() = true;
        b.points().push_back(point(324.75,-674.25));
        b.points().push_back(point(941.75,-326.55));
        b.points().push_back(point(-359.17,89.52));
        b.points().push_back(point(206.87,230.11));
        
        std::cout << "a: " << a << std::endl;
        a = db->query_value<path>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_polygon_test_bin)
{
    try{
        polygon p;
        
        std::cout << "p.bound_box().high().x(): " <<
            p.bound_box().high().x() << std::endl;
        p.bound_box().high().x() = 8.0;
        std::cout << "p.bound_box().high().x(): " <<
            p.bound_box().high().x() << std::endl;
        ASSERT_THAT(p.bound_box().high().x(), testing::Eq(8.0));
        
        point b = p.bound_box().high();
        point& c = p.bound_box().high();
        
        b.x() = 12.9;
        std::cout << "p.bound_box().high().x(): " <<
            p.bound_box().high().x() <<
            ", b.x(): " << b.x() <<
            std::endl;
        ASSERT_THAT(p.bound_box().high().x(), testing::Eq(8.0));
        
        c.x() = 12.9;
        std::cout << "p.bound_box().high().x(): " <<
            p.bound_box().high().x() <<
            ", c.x(): " << c.x() <<
            std::endl;
        ASSERT_THAT(p.bound_box().high().x(), testing::Eq(12.9));
        
        const polygon pc;
        pc.points().insert(pc.points().begin(), point(2, 3));
        std::cout << "pc size: " << pc.points().size() << std::endl;
        ASSERT_THAT((int)pc.points().size(), testing::Eq(0));
        
        polygon pd;
        pd.points().insert(pd.points().begin(), point(2, 3));
        std::cout << "pd size: " << pd.points().size() << std::endl;
        ASSERT_THAT((int)pd.points().size(), testing::Eq(1));
        
        
        auto d = db->query_value<polygon>(
            "select polygon '((324.75,-674.25),(941.75,-326.55),"
            "(-359.17,89.52),(206.87,230.11))'"
        );
        
        polygon e;
        e.points().push_back(point(324.75,-674.25));
        e.points().push_back(point(941.75,-326.55));
        e.points().push_back(point(-359.17,89.52));
        e.points().push_back(point(206.87,230.11));
        
        std::cout << "d: " << d << std::endl;
        d = db->query_value<polygon>(
            "select $1", e
        );
        std::cout << "d: " << d << std::endl;
        std::cout << "e: " << e << std::endl;
        ASSERT_THAT((std::string)d, testing::Eq((std::string)e));
        
    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}

TEST_F(geo_types_test, geo_circle_test_bin)
{
    try{
        auto a = db->query_value<circle>(
            "select '<(4, -11), 9>'::circle"
        );
        std::cout << "a: " << a << std::endl;
        auto b = a;
        a = db->query_value<circle>(
            "select $1", b
        );
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        ASSERT_THAT((std::string)a, testing::Eq((std::string)b));

    }catch(const std::exception& err){
        std::cout << "Error: " << err.what() << std::endl;
        FAIL();
    }
}


}} //namespace pq_async::tests