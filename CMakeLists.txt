cmake_minimum_required(VERSION 3.20)
project(Proyecto1_Paginacion)
set(CMAKE_CXX_STANDARD 17)

# Esto obliga a incluir las librerías dentro del .exe
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")

add_executable(generator src/generator.cpp)
add_executable(sorter src/sorter.cpp src/PagedArray.cpp src/PagedArray.h)