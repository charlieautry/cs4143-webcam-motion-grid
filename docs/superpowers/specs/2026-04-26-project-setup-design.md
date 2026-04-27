# Project Setup Design — Webcam-Driven 3D Pixel Grid

## Goal

Replace the course-provided Vermilion framework with a raw GLFW setup, integrate OpenCV and GLM, and establish the project structure for building the webcam-driven 3D pixel grid application.

## Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Framework | Raw GLFW (drop Vermilion) | Need mouse/scroll callbacks not exposed by Vermilion |
| OpenGL loader | gl3w (keep existing) | Already works, functionally identical to GLAD |
| Math library | GLM (bundled in repo) | Header-only, proper glm::mat4/lookAt/perspective support |
| OpenCV install | Manual download, pre-built binaries | Simplest for a course project |
| Grid dimensions | Configurable via GridConfig | Not hard-coded to 64x48 |

## Project Structure

```
cs4143-final-project/
  src/
    main.cpp            — GLFW window, GL context, render loop, input callbacks, OpenCV capture
    camera.h/.cpp       — Orbit camera (spherical coords, mouse drag/scroll)
    grid.h/.cpp         — GridConfig, mesh gen, instanced rendering, per-frame updates
    shader.h/.cpp       — Shader loading (vertex + fragment file paths -> program ID)
  external/
    glm/                — GLM headers (bundled, header-only)
  shaders/
    grid.vert           — Vertex shader (per-column height extrusion + Phong)
    grid.frag           — Fragment shader (Phong shading with per-column color)
  include/              — Existing course headers (gl3w, GLFW, GL3) — kept for gl3w
  lib/
    gl3w.c              — Kept, compiled directly
    glfw/               — Kept, built as subdirectory
  CMakeLists.txt        — Rewritten
```

## What Gets Removed

- **Vermilion library target** and all its sources (`vermilion.cpp`, `vbm.cpp`, `LoadShaders.cpp`, `loadtexture.cpp`, `targa.cpp`, `vdds.cpp`)
- **Vermilion headers**: `vapp.h`, `vgl.h`, `vmath.h`, `vbm.h`, `vermilion.h`, `vutils.h`, `vec.h`, `mat.h`
- **Old skeleton**: `src/final_project/final_project.cpp`
- **Prebuilt libs** that were only for Vermilion: `vermilion_d.lib`, etc.

Headers needed by gl3w are kept: `GL/glcorearb.h`, `GL/glext.h`, `GL3/gl3.h`, `GL3/gl3w.h`, and the GLFW headers via `lib/glfw/`.

## CMakeLists.txt Design

```cmake
cmake_minimum_required(VERSION 3.5)
project(cs4143-final-project)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

set(CMAKE_DEBUG_POSTFIX "_d")

# OpenGL
find_package(OpenGL REQUIRED)

# OpenCV — user must set OpenCV_DIR to <opencv>/build
find_package(OpenCV REQUIRED)

# GLFW (subdirectory build, already in repo)
add_subdirectory(lib/glfw)

# Include paths
include_directories(include)                  # gl3w, GL3 headers
include_directories(lib/glfw/include)         # GLFW headers
include_directories(external)                 # GLM (usage: #include <glm/glm.hpp>)
include_directories(${OpenCV_INCLUDE_DIRS})

# Executable
add_executable(final_project
    src/main.cpp
    src/camera.cpp
    src/grid.cpp
    src/shader.cpp
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
```

## OpenCV Setup (User Steps)

1. Download OpenCV pre-built binaries from https://opencv.org/releases/
2. Extract to a known location (e.g., `C:\opencv`)
3. Set `OpenCV_DIR` to `C:\opencv\build` (environment variable or CMake `-DOpenCV_DIR=...`)
4. Ensure `C:\opencv\build\x64\vc16\bin` (or equivalent) is on PATH so DLLs are found at runtime

## GLM Setup

1. Download GLM from the GitHub releases page
2. Extract so that `external/glm/glm/glm.hpp` exists
3. No build configuration needed — header-only

## Shader Loading (shader.h/.cpp)

Simplified from the course `LoadShaders.cpp`. Takes two file paths (vertex, fragment), reads them, compiles, links, returns a `GLuint` program ID. Reports errors to stderr. No framework types or macros.

```cpp
// shader.h
#pragma once
#include <GL3/gl3.h>
GLuint loadShaderProgram(const char* vertexPath, const char* fragmentPath);
```

## main.cpp Skeleton

- `glfwInit()`, create window (e.g., 1280x720), set OpenGL 3.3 core profile
- `gl3wInit()` to load GL function pointers
- Register GLFW callbacks: `glfwSetCursorPosCallback`, `glfwSetScrollCallback`, `glfwSetMouseButtonCallback`, `glfwSetKeyCallback`, `glfwSetFramebufferSizeCallback`
- Create `GridConfig` with defaults (64x48)
- Initialize `OrbitCamera` centered on the grid
- Open `cv::VideoCapture(0)`
- Compile shaders via `loadShaderProgram()`
- Initialize grid mesh and instance buffers
- Render loop: capture frame -> update grid -> render -> swap buffers -> poll events
- Cleanup on exit

## GridConfig

```cpp
struct GridConfig {
    int width     = 64;
    int height    = 48;
    float cellSize    = 1.0f;
    float maxHeight   = 5.0f;
    float sensitivity = 1.0f;
};
```

All systems (mesh generation, OpenCV downsampling, instance buffer allocation, camera framing) derive dimensions from this struct.

## Implementation Order

1. Download and configure OpenCV + GLM
2. Rewrite CMakeLists.txt, verify empty project builds
3. Write shader.h/.cpp
4. Write main.cpp skeleton (window + GL context + render loop showing a clear color)
5. Add OpenCV webcam capture, verify frames are grabbed
6. Write grid.h/.cpp with GridConfig, instanced box mesh
7. Write camera.h/.cpp with orbit controls
8. Wire it all together: webcam -> grid colors, mouse -> camera
9. Add motion detection + height extrusion (later phase)
10. Add Phong shading + modes (later phase)
