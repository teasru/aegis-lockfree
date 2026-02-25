cmake_minimum_required(VERSION 3.16)
project(aegis_lockfree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmark
    src/benchmark.cpp
)

target_include_directories(benchmark PRIVATE include)
