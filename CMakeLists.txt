cmake_minimum_required(VERSION 3.5)
project(cs4143-final-project)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

# OpenGL
find_package(OpenGL REQUIRED)

# OpenCV — set OpenCV_DIR to <opencv>/build if not found automatically
find_package(OpenCV REQUIRED)

# GLFW (subdirectory build)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

# Include paths
include_directories(include)              # gl3w, GL3 headers
include_directories(lib/glfw/include)     # GLFW headers
include_directories(external/glm)         # GLM: #include <glm/glm.hpp>
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(src)                  # project headers

# Executable
add_executable(final_project
    src/main.cpp
    src/shader.cpp
    src/camera.cpp
    src/grid.cpp
    src/text.cpp
    lib/gl3w.c
)

target_link_libraries(final_project
    ${OPENGL_LIBRARIES}
    glfw
    ${OpenCV_LIBS}
)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTY VS_STARTUP_PROJECT final_project)
endif()
