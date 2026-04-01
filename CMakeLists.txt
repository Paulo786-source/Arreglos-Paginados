cmake_minimum_required(VERSION 3.20)
project(Proyecto1_Paginacion)
set(CMAKE_CXX_STANDARD 17)

add_executable(generator generator.cpp)
add_executable(sorter sorter.cpp PagedArray.cpp PagedArray.h)