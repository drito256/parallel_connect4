cmake_minimum_required(VERSION 3.20)

project(connect4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(connect4
    src/main.cpp
    src/grid.cpp
    src/cursor.cpp
    src/input.cpp
    src/connect4.cpp
)

target_include_directories(connect4
    PUBLIC include
)