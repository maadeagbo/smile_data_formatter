cmake_minimum_required(VERSION 3.5.1)
project (Face_Keras_Exporter)

# set release or debug builds
if(CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_CONFIGURATION_TYPES Release RelWithDebInfo Debug)
   set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
     "Reset the configurations to what we need"
     FORCE)
 endif()
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/bin )
		set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/lib )
		set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} 
				${PROJECT_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# set flags for debug and release mode
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-g HAS_DEBUG)
check_cxx_compiler_flag(-Wall HAS_WALL)
check_cxx_compiler_flag(-O0 HAS_OPT0)
check_cxx_compiler_flag(-O3 HAS_OPT3)
check_cxx_compiler_flag(-fmessage-length=0 HAS_MSG)
check_cxx_compiler_flag(-std=c++11 HAS_CNEW)
check_cxx_compiler_flag(-ldl HAS_LDL)
if(HAS_DEBUG)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()
if(HAS_WALL)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif()
if(HAS_OPT0)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
endif()
if(HAS_OPT3)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()
if(HAS_MSG)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")
endif()
if(HAS_CNEW)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
endif()
if(UNIX)
    if(NOT HAS_LDL)
        message(FATAL_ERROR "-ldl not found in system")
    endif()
endif()

# include directories for project
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/glm)

# use the file(GLOB...) or file(GLOB_RECURSE...) to grab wildcard src files
file(GLOB_RECURSE SOURCES 	"${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE INCLUDES 	"${CMAKE_SOURCE_DIR}/include/*.h")

add_executable(fk_data ${SOURCES} ${INCLUDES})

# set visual studio startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fk_data)

if(UNIX)
	find_library(FS_LIB stdc++fs /usr/lib/gcc/x86_64-linux-gnu/*)
	if(NOT FS_LIB)
		message(FATAL_ERROR "C++17 File System library not found")
	endif()
endif()

if(NOT WIN32)
	# filesystem goes last
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -lstdc++fs")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -lstdc++fs")

	target_link_libraries(fk_data ${FS_LIB})
endif()