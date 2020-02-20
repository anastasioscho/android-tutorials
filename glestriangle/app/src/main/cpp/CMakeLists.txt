cmake_minimum_required(VERSION 3.6.0)

add_library(my-library SHARED main.cpp)

find_library(opengl-lib GLESv3)

target_link_libraries(my-library ${opengl-lib})
