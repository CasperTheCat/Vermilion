cmake_minimum_required(VERSION 3.5)
project(Vermilion)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        core/engines/renderEngine.cpp
        core/engines/renderEngine.h
        core/engines/meshEngine.cpp
        core/engines/meshEngine.h
        core/engines/loggingEngine.cpp
        core/engines/loggingEngine.h
        core/frameTile/frameTile.cpp
        core/frameTile/frameTile.h core/types/types.h)


link_directories(${CMAKE_SOURCE_DIR}/assimp/lib)
add_executable(Vermilion ${SOURCE_FILES})
target_link_libraries(Vermilion assimp-vc120-mt.lib)
