cmake_minimum_required(VERSION 3.22)
project(Coroutine)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wextra")
set (CMAKE_ASM-ATT_FLAGS "${CMAKE_ASM_FLAGS} -g")

include_directories(.)

add_executable(test
        test.cpp)

enable_language(ASM-ATT)
set_property(SOURCE coroutine_operation.S PROPERTY LANGUAGE ASM-ATT)
target_sources(test PRIVATE coroutine_operation.S)