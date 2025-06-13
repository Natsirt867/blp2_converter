cmake_minimum_required(VERSION 3.31)
project(blp2 C)

set(CMAKE_C_STANDARD 11)

add_executable(blp2 main.c
        type_headers.h)
