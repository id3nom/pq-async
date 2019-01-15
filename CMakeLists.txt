cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0054 NEW)
project(pq-async VERSION 0.1.0 LANGUAGES CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# define module path
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

if(NOT PrecompiledHeader_CMake_INCLUDED)
    include(PrecompiledHeader)
endif()

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
set(PQ_ASYNC_BUILD_TYPE "DEBUG" CACHE STRING "pq-async BUILD TYPE")
set(PQ_ASYNC_BUILD_TYPE_VALUES "DEBUG;TEST;SHIPPING;")
set_property(CACHE PQ_ASYNC_BUILD_TYPE PROPERTY STRINGS ${PQ_ASYNC_BUILD_TYPE_VALUES})

message(STATUS "pq-async build type='${PQ_ASYNC_BUILD_TYPE}'")
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

option(PQ_ASYNC_BUILD_TESTS "Build tests" OFF)
option(PQ_ASYNC_BUILD_DOC "Build documentation" OFF)

#############
# add subdirs

# deps
if(NOT TARGET gmock)
    subdirs(deps/googletest)
endif()
if(NOT TARGET tz)
    subdirs(deps/date)
endif()

# lib dir
#subdirs(src)

# tests
if(PQ_ASYNC_BUILD_TESTS)
    subdirs(tests)
    add_test(alltests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pq-async_tests)
    enable_testing()
endif()
if(PQ_ASYNC_BUILD_TESTS)
    subdirs(doc)
endif()

#######
# cpack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
if(NOT CPack_CMake_INCLUDED)
    include(CPack)
endif()







#####################
### pq-async ###
#####################

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/pq_async_config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pq-async/pq_async_config.h"
)

set(PostgreSQL_ROOT "" CACHE PATH "Set the PostgreSQL_ROOT system variable to where PostgreSQL is found on the machine E.g C:/Program Files/PostgreSQL/VERSION")
#set(PostgreSQL_TYPE_INCLUDE_DIR "/usr/include/postgresql/" CACHE PATH "sets the debian postgresql include dir to fix a bug in postgresl")

# build thread safe components
option(PQ_ASYNC_THREAD_SAFE "Enable thread safety." ON)

include(FindPostgreSQL)
Find_Package(PostgreSQL REQUIRED)

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
#    "${PROJECT_SOURCE_DIR}/src"
#    "${PROJECT_SOURCE_DIR}/include/pq-async"
#    "${PROJECT_SOURCE_DIR}/deps/date/include"
#    "${PROJECT_SOURCE_DIR}/deps/fmt/include"
#    "${PROJECT_SOURCE_DIR}/deps/json/single_include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pq-async"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/date/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/fmt/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/json/single_include"

    ${PostgreSQL_INCLUDE_DIRS}
)
#link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
file(GLOB PQ_ASYNC_HEADERS "${PROJECT_SOURCE_DIR}/include/pq-async/*.h")

source_group("Headers" FILES ${PQ_ASYNC_HEADERS})

file(GLOB PQ_ASYNC_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

include(GenerateExportHeader)
#add_compiler_export_flags() # deprecated
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
if(${BUILD_SHARED_LIBS})
    add_library(pq-async SHARED
        ${PQ_ASYNC_SOURCES}
        ${PQ_ASYNC_HEADERS}
    )
else()
    add_library(pq-async STATIC
        ${PQ_ASYNC_SOURCES}
        ${PQ_ASYNC_HEADERS}
    )
endif()
set_target_properties(pq-async PROPERTIES LINKER_LANGUAGE CXX)

generate_export_header(pq-async
    EXPORT_MACRO_NAME "PQ_ASYNC_CLIENT_API"
    EXPORT_FILE_NAME "${PROJECT_SOURCE_DIR}/include/pq-async/pq_asyncpp_export.h"
)

if(WIN32)
   set_target_properties(pq-async PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(pq-async PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   #set_target_properties(pq-async PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   #set_target_properties(pq-async PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
   set_target_properties(pq-async PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
endif(WIN32)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #add_precompiled_header(xpzs ServerPCH.h 
    #   FORCEINCLUDE
    #   #SOURCE_CXX:CorePrivatePCH.cpp
    #   SOURCE_CXX "${PROJECT_SOURCE_DIR}/server/src/ServerPCH.cpp"
    #)

    # add required windows compile flags
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Zm200 -wd4503 -bigobj " )
    
    # add required windows libs
    if(${PQ_ASYNC_BUILD_DEBUG})
        TARGET_LINK_LIBRARIES(pq-async Ws2_32.lib Setupapi.lib Dbghelp.lib Winmm.lib Iphlpapi.lib Netapi32.lib Imm32.lib Xinput.lib Dwmapi.lib Wininet.lib )
        
    else(${PQ_ASYNC_BUILD_DEBUG})
        TARGET_LINK_LIBRARIES(pq-async Ws2_32.lib Setupapi.lib Dbghelp.lib Winmm.lib Iphlpapi.lib Netapi32.lib Imm32.lib Dwmapi.lib Xinput.lib Wininet.lib )
        
    endif(${PQ_ASYNC_BUILD_DEBUG})

else(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    #SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${PROJECT_SOURCE_DIR}/server/inc/ServerPCH.h" )
    
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

target_link_libraries(pq-async 
    ${Boost_LIBRARIES} 
    "${PostgreSQL_LIBRARY}"
    uuid
)
