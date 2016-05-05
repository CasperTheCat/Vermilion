cmake_minimum_required(VERSION 3.5)
project(Vermilion)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp core/renderEngine.cpp core/renderEngine.h core/frameTile/frameTile.cpp core/frameTile/frameTile.h)
add_executable(Vermilion ${SOURCE_FILES})