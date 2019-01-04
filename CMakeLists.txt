cmake_minimum_required(VERSION 3.7)
project(Vermilion)

#set(Boost_USE_STATIC_LIBS TRUE)
#set(Boost_USE_MULTITHREADED TRUE)
#set(Boost_USE_STATIC_RUNTIME FALSE)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -Wall -ggdb")
#FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
#FIND_PACKAGE(OpenEXR)
FIND_PACKAGE(OpenImageIO REQUIRED)
FIND_PACKAGE(Assimp REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(Vulkan)

set(USE_GUI true CACHE BOOL "Disable GUI")
if (Vulkan_FOUND OR USE_GUI)
    add_definitions(-DUseGUI)
    
endif()

#FIND_PACKAGE(OpenImageIO)

set(SOURCE_FILES
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
        core/integrators/pathtracer.cpp
        core/integrators/integrators.h
        core/accelerators/bvh.cpp
        core/accelerators/bvh.h
        core/accelerators/bbox.cpp
        core/accelerators/bbox.h
        core/accelerators/triangle.cpp
        core/accelerators/triangle.h
        core/accelerators/IntersectionInfo.h
        core/accelerators/object.h
        core/accelerators/Ray.h
        core/renderer/renderer.cpp
        core/renderer/renderer.h
        core/renderer/tools.h
        )

set(FRONT_END
    main.cpp
    extern/imgui/imgui.h
    extern/imgui/imgui_internal.h
    extern/imgui/imstb_rectpack.h
    extern/imgui/imstb_textedit.h
    extern/imgui/imstb_truetype.h
    extern/imgui/imgui.cpp
    extern/imgui/imgui_demo.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_widgets.cpp
)

include_directories(
        ${Vulkan_INCLUDE_DIRS}
        ${OIIO_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/core
        ${CMAKE_SOURCE_DIR}/extern
        ${CMAKE_SOURCE_DIR}/extern/glfw/include
)

add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glfw)

#link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(Vermilion ${FRONT_END} ${SOURCE_FILES})

target_link_libraries(Vermilion
    glfw
    ${ASSIMP_LIBRARIES}
    ${Boost_LIBRARIES} 
    ${OIIO_LIBRARIES}
    ${Vulkan_LIBRARIES}
    ${GLFW_LIBRARIES}
)

