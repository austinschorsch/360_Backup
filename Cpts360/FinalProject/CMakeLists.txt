cmake_minimum_required(VERSION 3.3)
project(FinalProject)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES diskimage mountroot.c mountroot.h type.h main.c)
add_executable(FinalProject ${SOURCE_FILES})