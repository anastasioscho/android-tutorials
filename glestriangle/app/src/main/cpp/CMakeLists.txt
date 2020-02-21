cmake_minimum_required(VERSION 3.6.0)

add_library(native-library SHARED main.cpp)

find_library(opengl-lib GLESv3)
find_library(log-lib log)

target_link_libraries(native-library ${opengl-lib} ${log-lib})
