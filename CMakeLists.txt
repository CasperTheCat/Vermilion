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


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -Ofast -Wall -ggdb")
#FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
#FIND_PACKAGE(OpenEXR)
FIND_PACKAGE(OpenImageIO REQUIRED)

set(SOURCE_FILES
        main.cpp
        core/engines/renderEngine.cpp
        core/engines/meshEngine.cpp
        core/engines/loggingEngine.cpp
        core/camera/frameTile/frameTile.cpp
        core/camera/camera.cpp
        core/integrators/integrators.cpp
        core/nvidia/BVH.cpp
        core/nvidia/BVHNode.cpp
        core/nvidia/CudaBVH.cpp
        core/nvidia/SplitBVHBuilder.cpp
        core/nvidia/Util.cpp
        core/nvidia/Sort.cpp
        )

set(HEADER_FILES 
        core/integrators/integrators.h
        core/camera/frameTile/frameTile.h
        core/engines/meshEngine.h
        core/engines/renderEngine.h
        core/types/types.h
        core/engines/loggingEngine.h
        core/camera/camera.h
        core/nvidia/Array.h
        core/nvidia/BVH.h
        core/nvidia/BVHNode.h
        core/nvidia/CudaBVH.h
        core/nvidia/linear_math.h
        core/nvidia/Scene.h
        core/nvidia/SplitBVHBuilder.h 
        core/integrators/integratorConstants.h
        core/nvidia/Util.h
        core/nvidia/Sort.h
        )

include_directories(
        ${Boost_INCLUDE_DIRS}
        ${OIIO_INCLUDE_DIRS}
)

link_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(Vermilion ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Vermilion ${CMAKE_SOURCE_DIR}/lib/libassimp.so.3.3.1 ${Boost_LIBRARIES} ${OIIO_LIBRARIES}
    )
