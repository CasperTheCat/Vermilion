cmake_minimum_required(VERSION 2.8)
project(Vermilion)

set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_USE_STATIC_RUNTIME FALSE)

find_package(OpenMP) 
if (OPENMP_FOUND) 
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}") 
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}") 
endif() 


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -Ofast -Wall -ggdb")
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
FIND_PACKAGE(OpenEXR)
FIND_PACKAGE(OpenImageIO REQUIRED)

set(SOURCE_FILES main.cpp
        core/engines/renderEngine.cpp
        core/engines/renderEngine.h
        core/engines/meshEngine.cpp
        core/engines/meshEngine.h
        core/engines/loggingEngine.cpp
        core/engines/loggingEngine.h
        core/camera/frameTile/frameTile.cpp
        core/camera/frameTile/frameTile.h
        core/types/types.h
        core/camera/camera.cpp
        core/camera/camera.h
        core/integrators/integrators.cpp
        core/integrators/integrators.h
        )

include_directories(
        ${Boost_INCLUDE_DIRS}
        ${OIIO_INCLUDE_DIRS}
)

link_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(Vermilion ${SOURCE_FILES})
target_link_libraries(Vermilion ${CMAKE_SOURCE_DIR}/lib/libassimp.so.3.3.1 ${Boost_LIBRARIES} ${OIIO_LIBRARIES}
    /usr/lib64/libpng.so
    )
