cmake_minimum_required(VERSION 3.5)

project(JuliaRender LANGUAGES CXX)

add_executable(JuliaRender main.cpp ocl_render.cpp)
target_link_libraries(JuliaRender SDL2 m OpenCL)
