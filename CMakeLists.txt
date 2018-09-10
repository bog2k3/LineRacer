# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

# Project's name

project(LineRacer)
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

# The following folder will be included
#include_directories("${BOGLFW_DIST_DIR}/include")
link_directories("/mingw/lib")
include_directories("/mingw/include")

# add sources
file(GLOB_RECURSE sources	src/*.h)
file(GLOB_RECURSE sources 	src/*.cpp)

add_executable(LineRacer ${sources})

# add explicit dependency on boglfw library to force the target to be re-linked when the library is changed.
# for some reason CMake doesn't do this automatically even though we do depend on the library.
#set_property(TARGET LineRacer PROPERTY LINK_DEPENDS "${BOGLFW_DIST_DIR}/lib/libboglfw.a")

#add libraries
target_link_libraries(LineRacer PUBLIC SDL2)

set_property(TARGET LineRacer PROPERTY CXX_STANDARD 14)
target_compile_options(LineRacer PUBLIC -Wall -std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
