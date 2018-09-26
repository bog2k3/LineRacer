# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name
project(LineRacer)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

# The following folder will be included
if (WIN32)
	link_directories("/mingw/lib")
	include_directories("/mingw/include")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WIN32__")
else()
	link_directories("/usr/lib")
	include_directories("/usr/include")
endif()

# add sources
file(GLOB_RECURSE sources	src/*.h)
file(GLOB_RECURSE sources 	src/*.cpp)

add_executable(LineRacer ${sources})

#add libraries
target_link_libraries(LineRacer PUBLIC SDL2)
if (WIN32)
	target_link_libraries(LineRacer PUBLIC ws2_32)
else()
	target_link_libraries(LineRacer PUBLIC pthread)
endif()

set_property(TARGET LineRacer PROPERTY CXX_STANDARD 14)
target_compile_options(LineRacer PUBLIC -Wall -std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
