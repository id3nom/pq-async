cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0054 NEW)
project(libpq-async++ VERSION 0.1.0 LANGUAGES CXX C)

# define module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")

include(PrecompiledHeader)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# not using unicode at all, better using UTF-8
# add_definitions(-DUNICODE -D_UNICODE)

if(UNIX)
	message(STATUS " Setting GCC flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -g -Wall -Werror -fmax-errors=10 -std=gnu++14 -msse4.1")
	
else(UNIX)
	message(STATUS " Setting MSVC flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /arch:SSE2")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /VERBOSE:LIB ")
	
endif(UNIX)
message(STATUS "** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# boost dependencies
set(Boost_ADDITIONAL_VERSIONS "1.62" "1.58" "1.58.0" "1.57" "1.57.0" "1.56" "1.56.0" "1.55" "1.55.0")

#set(Boost_ROOT "" CACHE PATH "Boost root dir")
set(Boost_DEBUG  ON CACHE BOOL "Boost_DEBUG")
set(Boost_USE_STATIC_LIBS        ON CACHE BOOL "Boost_USE_STATIC_LIBS")
set(Boost_USE_MULTITHREADED      ON CACHE BOOL "Boost_USE_MULTITHREADED")
set(Boost_USE_STATIC_RUNTIME     OFF CACHE BOOL "Boost_USE_STATIC_RUNTIME")
set(Boost_ALL_DYN_LINK           OFF CACHE BOOL "BOOST_ALL_DYN_LINK") 

set(PQ_ASYNC_BOOST_COMPONENTS filesystem atomic date_time system iostreams regex program_options thread chrono locale)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Boost COMPONENTS ${PQ_ASYNC_BOOST_COMPONENTS} )
message(STATUS "** BOOST Include: ${Boost_INCLUDE_DIR}")
message(STATUS "** BOOST Libraries Dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "** BOOST Libraries: ${Boost_LIBRARIES}")

# static or shared build configuration
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
if(${BUILD_SHARED_LIBS})
	set(IS_MONOLITHIC 0)
else()
	set(IS_MONOLITHIC 1)	
endif()


# debug vs test vs shipping build type
set(PQ_ASYNC_BUILD_TYPE "DEBUG" CACHE STRING "libpq-async++ BUILD TYPE")
set(PQ_ASYNC_BUILD_TYPE_VALUES "DEBUG;TEST;SHIPPING;")
set_property(CACHE PQ_ASYNC_BUILD_TYPE PROPERTY STRINGS ${PQ_ASYNC_BUILD_TYPE_VALUES})

message(STATUS "libpq-async++ build type='${PQ_ASYNC_BUILD_TYPE}'")
if( ${PQ_ASYNC_BUILD_TYPE} STREQUAL "DEBUG")
	set(PQ_ASYNC_BUILD_DEBUG 1)
elseif( ${PQ_ASYNC_BUILD_TYPE} STREQUAL "TEST")
	set(PQ_ASYNC_BUILD_TEST 1)
else(${PQ_ASYNC_BUILD_TYPE} STREQUAL "DEBUG")
	set(PQ_ASYNC_BUILD_SHIPPING 1)
endif(${PQ_ASYNC_BUILD_TYPE} STREQUAL "DEBUG")

set(BITNESS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BITNESS 64)
endif()

#############
# add subdirs

# deps
subdirs(deps/gmock-1.7.0)
subdirs(deps/date)


# lib dir
subdirs(src tests doc)

# tests
add_test(alltests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pq-async++_tests)
option(WITH_TEST "build test" ON)
if(WITH_TEST)
	enable_testing()
endif()

#######
# cpack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)