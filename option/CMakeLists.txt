cmake_minimum_required(VERSION 3.5)

project(option)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(
    mcver
    src/mcver.cpp
)

target_link_libraries(
    mcver
    ncurses
)

SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(
    -Wall
    -g
    -O0
    -fsanitize=address
)