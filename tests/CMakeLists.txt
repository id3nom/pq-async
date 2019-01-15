configure_file (
    "${PROJECT_SOURCE_DIR}/pq_asyncpp_config.h.in"
    "${PROJECT_BINARY_DIR}/pq_asyncpp_config.h"
)

set(PostgreSQL_ROOT "" CACHE PATH "Set the PostgreSQL_ROOT system variable to where PostgreSQL is found on the machine E.g C:/Program Files/PostgreSQL/VERSION")
include(FindPostgreSQL)
Find_Package(PostgreSQL REQUIRED)

include_directories("${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include/"
    ${PostgreSQL_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/deps/date/include"
    "${PROJECT_SOURCE_DIR}/deps/fmt/include"
    "${PROJECT_SOURCE_DIR}/deps/json/single_include"
    "${PROJECT_SOURCE_DIR}/deps/googletest/googletest/include"
    "${PROJECT_SOURCE_DIR}/deps/googletest/googlemock/include"
)

set(test_sources
    main.cpp
    type_tests/bin_types_test.cpp
    type_tests/char_types_test.cpp
    type_tests/cash_types_test.cpp
    type_tests/date_types_test.cpp
    type_tests/enum_types_test.cpp
    type_tests/geo_types_test.cpp
    type_tests/net_types_test.cpp
    type_tests/num_types_test.cpp
    type_tests/range_types_test.cpp
    type_tests/sys_types_test.cpp
    type_tests/uuid_types_test.cpp
    queue_tests/queue_test.cpp
    queue_tests/cb_test.cpp
    db_tests/data_reader_test.cpp
    db_tests/data_prepared_test.cpp
    db_tests/database_test.cpp
)
add_executable(pq-async_tests ${test_sources})


target_link_libraries(pq-async_tests
    pq-async
    tz
    ${Boost_LIBRARIES} 
    "${PostgreSQL_LIBRARY}"
    pthread
    gmock
#	gtest
)
