cmake_minimum_required(VERSION 3.10)
project(p2)
find_package(OpenCV REQUIRED)
add_executable(p2 ex2.cpp)
target_link_libraries(p2 ${OpenCV_LIBS})