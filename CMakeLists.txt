cmake_minimum_required(VERSION 2.8)
project(Vermilion)

set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_USE_STATIC_RUNTIME FALSE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)

set(SOURCE_FILES main.cpp
        core/engines/renderEngine.cpp
        core/engines/renderEngine.h
        core/engines/meshEngine.cpp
        core/engines/meshEngine.h
        core/engines/loggingEngine.cpp
        core/engines/loggingEngine.h
        core/camera/frameTile/frameTile.cpp
        core/camera/frameTile/frameTile.h
        core/types/types.h core/camera/camera.cpp core/camera/camera.h core/integrators/integrators.cpp core/integrators/integrators.h)

include_directories(
        ${Boost_INCLUDE_DIRS}
)

link_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(Vermilion ${SOURCE_FILES})
target_link_libraries(Vermilion ${CMAKE_SOURCE_DIR}/lib/libassimp.so.3.3.1 ${Boost_LIBRARIES}
    /usr/lib64/libpng.so
    )
