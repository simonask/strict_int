cmake_minimum_required(VERSION 3.8)
project(StringInt VERSION 0.0.1)
# Project-wide build flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(src/strict_int)

enable_testing()
add_subdirectory(test)
