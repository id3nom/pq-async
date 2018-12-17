# libpq-async++
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/pq_asyncpp_config.h.in"
	"${PROJECT_BINARY_DIR}/pq_asyncpp_config.h"
)

set(PostgreSQL_ROOT "" CACHE PATH "Set the PostgreSQL_ROOT system variable to where PostgreSQL is found on the machine E.g C:/Program Files/PostgreSQL/VERSION")
#set(PostgreSQL_TYPE_INCLUDE_DIR "/usr/include/postgresql/" CACHE PATH "sets the debian postgresql include dir to fix a bug in postgresl")

# build thread safe components
option(PQ_ASYNC_THREAD_SAFE "Enable thread safety." ON)

include(FindPostgreSQL)
Find_Package(PostgreSQL REQUIRED)

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/deps/date/include"
	${PostgreSQL_INCLUDE_DIRS}
)
#link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
file(GLOB PQ_ASYNC_HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
# list(APPEND PQ_ASYNC_HEADERS
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decNumber.h"
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decContext.h"
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decimal64.h"
# )

source_group("Headers" FILES ${PQ_ASYNC_HEADERS})

file(GLOB PQ_ASYNC_SOURCES *.cpp)
# list(APPEND PQ_ASYNC_SOURCES
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decNumber.c"
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decContext.c"
# 	"${PROJECT_SOURCE_DIR}/src/decNumber/decimal64.c"
# )

include(GenerateExportHeader)
#add_compiler_export_flags() # deprecated
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
if(${BUILD_SHARED_LIBS})
	add_library(pq-async++ SHARED
		${PQ_ASYNC_SOURCES}
		${PQ_ASYNC_HEADERS}
	)
else()
	add_library(pq-async++ STATIC
		${PQ_ASYNC_SOURCES}
		${PQ_ASYNC_HEADERS}
	)
endif()
set_target_properties(pq-async++ PROPERTIES LINKER_LANGUAGE CXX)

generate_export_header(pq-async++
	EXPORT_MACRO_NAME "PQ_ASYNC_CLIENT_API"
	EXPORT_FILE_NAME "${PROJECT_SOURCE_DIR}/src/pq_asyncpp_export.h"
)

if(WIN32)
   set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async++ PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async++ PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
   #set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   #set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
   set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
   set_target_properties(pq-async++ PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
endif(WIN32)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#add_precompiled_header(xpzs ServerPCH.h 
	#	FORCEINCLUDE
	#	#SOURCE_CXX:CorePrivatePCH.cpp
	#	SOURCE_CXX "${PROJECT_SOURCE_DIR}/server/src/ServerPCH.cpp"
	#)

	# add required windows compile flags
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Zm200 -wd4503 -bigobj " )
	
	# add required windows libs
	if(${PQ_ASYNC_BUILD_DEBUG})
		TARGET_LINK_LIBRARIES(pq-async++ Ws2_32.lib Setupapi.lib Dbghelp.lib Winmm.lib Iphlpapi.lib Netapi32.lib Imm32.lib Xinput.lib Dwmapi.lib Wininet.lib )
		
	else(${PQ_ASYNC_BUILD_DEBUG})
		TARGET_LINK_LIBRARIES(pq-async++ Ws2_32.lib Setupapi.lib Dbghelp.lib Winmm.lib Iphlpapi.lib Netapi32.lib Imm32.lib Dwmapi.lib Xinput.lib Wininet.lib )
		
	endif(${PQ_ASYNC_BUILD_DEBUG})

else(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${PROJECT_SOURCE_DIR}/server/inc/ServerPCH.h" )
	
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

target_link_libraries(pq-async++ 
	${Boost_LIBRARIES} 
	"${PostgreSQL_LIBRARY}"
	uuid
)
