cmake_minimum_required(VERSION 3.27)
project(CppDemo)

add_definitions(-DTIMECOUNT)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(./src)

add_executable(CppDemo main.cpp)
