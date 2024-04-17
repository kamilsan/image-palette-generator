cmake_minimum_required(VERSION 3.10)
project(palette)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(HEADER_FILES
    include/Color.hpp
    include/RNG.hpp
    include/Image.hpp
    include/KMeansClustering.hpp)

set(SOURCE_FILES
    src/KMeansClustering.cpp
    src/Image.cpp
    src/main.cpp)

add_custom_target(
    clang-format
    COMMAND clang-format -i ${HEADER_FILES} ${SOURCE_FILES}
)

include_directories(include 3rd_party)

add_executable(palette ${SOURCE_FILES})