cmake_minimum_required(VERSION 3.25)
project(CSE340_Project1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(CSE340_Project1
        inputbuf.cc
        inputbuf.h
        lexer.cc
        lexer.h types.h debug.h)
