cmake_minimum_required (VERSION 2.6)
project (myjson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(myjson myjson.c)
add_executable(myjson_test test.c)
target_link_libraries(myjson_test myjson)
