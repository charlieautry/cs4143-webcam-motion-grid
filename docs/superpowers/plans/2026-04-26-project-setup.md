# Project Setup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the Vermilion framework with raw GLFW, integrate OpenCV and GLM, and build a working skeleton that opens a window, captures webcam frames, and renders an instanced colored grid with an orbit camera.

**Architecture:** Raw GLFW main loop with gl3w for OpenGL loading. OpenCV captures webcam frames and downsamples to grid resolution. Grid is rendered as instanced box columns with colors from webcam pixels. Orbit camera controlled by mouse.

**Tech Stack:** C++, OpenGL 3.3 (gl3w), GLFW, GLM (bundled), OpenCV (pre-built)

---

## File Structure

| Action | Path | Responsibility |
|--------|------|----------------|
| Create | `src/main.cpp` | GLFW window, GL context, render loop, input callbacks, OpenCV capture |
| Create | `src/shader.h` | `loadShaderProgram()` declaration |
| Create | `src/shader.cpp` | Read shader files, compile, link, error reporting |
| Create | `src/camera.h` | `OrbitCamera` struct declaration |
| Create | `src/camera.cpp` | Orbit camera: spherical coords, view matrix, mouse/scroll handling |
| Create | `src/grid.h` | `GridConfig`, `Grid` class declaration |
| Create | `src/grid.cpp` | Box mesh generation, instance buffer, per-frame color update |
| Create | `bin/media/shaders/grid/grid.vert` | Vertex shader: per-instance position, height, color |
| Create | `bin/media/shaders/grid/grid.frag` | Fragment shader: solid per-column color (Phong added later) |
| Create | `external/glm/` | GLM headers (manual download) |
| Modify | `CMakeLists.txt` | Rewrite for new structure |
| Delete | `src/final_project/final_project.cpp` | Old Vermilion skeleton |

---

### Task 0: Install Dependencies (User Steps)

These are manual steps the user performs before any code tasks.

- [ ] **Step 1: Download and extract OpenCV**

Download OpenCV 4.x pre-built binaries for Windows from https://opencv.org/releases/. Extract to `C:\opencv` (or another location). The result should have `C:\opencv\build\OpenCVConfig.cmake`.

- [ ] **Step 2: Add OpenCV DLLs to PATH**

Add `C:\opencv\build\x64\vc16\bin` (adjust `vc16` to match your VS version) to the system PATH so the application can find OpenCV DLLs at runtime.

- [ ] **Step 3: Download and place GLM**

Download GLM from https://github.com/g-truc/glm/releases. Extract so the headers live at `external/glm/glm/glm.hpp` inside the project.

Verify:
```
cs4143-final-project/external/glm/glm/glm.hpp   <-- this file must exist
```

- [ ] **Step 4: Verify both are in place**

```bash
ls external/glm/glm/glm.hpp
ls /c/opencv/build/OpenCVConfig.cmake
```

Both should exist before proceeding.

---

### Task 1: Rewrite CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Replace CMakeLists.txt contents**

```cmake
cmake_minimum_required(VERSION 3.5)
project(cs4143-final-project)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

set(CMAKE_DEBUG_POSTFIX "_d")

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

- [ ] **Step 2: Create stub source files so CMake can configure**

Create these minimal stubs so the build system doesn't fail on missing files:

`src/shader.h`:
```cpp
#pragma once
```

`src/shader.cpp`:
```cpp
#include "shader.h"
```

`src/camera.h`:
```cpp
#pragma once
```

`src/camera.cpp`:
```cpp
#include "camera.h"
```

`src/grid.h`:
```cpp
#pragma once
```

`src/grid.cpp`:
```cpp
#include "grid.h"
```

`src/main.cpp`:
```cpp
int main() {
    return 0;
}
```

- [ ] **Step 3: Regenerate the build and verify it compiles**

```bash
cd build
cmake .. -DOpenCV_DIR=C:/opencv/build
cmake --build . --config Debug
```

Expected: builds successfully with no errors. The `bin/` directory should contain `final_project_d.exe`.

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt src/main.cpp src/shader.h src/shader.cpp src/camera.h src/camera.cpp src/grid.h src/grid.cpp
git commit -m "Replace Vermilion framework with raw GLFW + OpenCV + GLM build"
```

---

### Task 2: Shader Loader

**Files:**
- Create: `src/shader.h`
- Create: `src/shader.cpp`

- [ ] **Step 1: Write shader.h**

```cpp
#pragma once
#include <GL3/gl3w.h>

// Loads vertex and fragment shader files, compiles and links them.
// Returns the program ID, or 0 on failure (errors printed to stderr).
GLuint loadShaderProgram(const char* vertexPath, const char* fragmentPath);
```

- [ ] **Step 2: Write shader.cpp**

```cpp
#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

static GLuint compileShader(GLenum type, const char* source, const char* path) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error (" << path << "): " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint loadShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertSrc = readFile(vertexPath);
    std::string fragSrc = readFile(fragmentPath);
    if (vertSrc.empty() || fragSrc.empty()) return 0;

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str(), vertexPath);
    if (!vert) return 0;

    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str(), fragmentPath);
    if (!frag) {
        glDeleteShader(vert);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}
```

- [ ] **Step 3: Verify it compiles**

```bash
cd build && cmake --build . --config Debug
```

Expected: no errors.

- [ ] **Step 4: Commit**

```bash
git add src/shader.h src/shader.cpp
git commit -m "Add shader loading utility"
```

---

### Task 3: GLFW Window + OpenGL Context

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Write main.cpp with window creation and clear-color render loop**

```cpp
#include <GL3/gl3w.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"
#include "camera.h"
#include "grid.h"

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "Webcam Pixel Grid", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize gl3w
    if (gl3wInit()) {
        std::cerr << "Failed to initialize gl3w" << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
```

- [ ] **Step 2: Build and run**

```bash
cd build && cmake --build . --config Debug
cd ../bin && ./final_project_d.exe
```

Expected: a dark gray window titled "Webcam Pixel Grid" that stays open until closed. No crashes.

- [ ] **Step 3: Commit**

```bash
git add src/main.cpp
git commit -m "Add GLFW window with OpenGL 3.3 context"
```

---

### Task 4: Orbit Camera

**Files:**
- Create: `src/camera.h`
- Create: `src/camera.cpp`

- [ ] **Step 1: Write camera.h**

```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct OrbitCamera {
    glm::vec3 target = glm::vec3(0.0f);
    float distance = 50.0f;
    float yaw   = 0.0f;    // radians
    float pitch  = 0.6f;   // radians, ~34 degrees looking down
    glm::vec3 panOffset = glm::vec3(0.0f);

    // Sensitivity
    float rotateSensitivity = 0.005f;
    float zoomSensitivity   = 2.0f;
    float panSensitivity    = 0.05f;

    // Pitch limits
    float minPitch = -1.5f;  // ~-86 degrees
    float maxPitch =  1.5f;  // ~86 degrees

    // Zoom limits
    float minDistance = 5.0f;
    float maxDistance = 200.0f;

    glm::vec3 getPosition() const;
    glm::mat4 getViewMatrix() const;

    void rotate(float dx, float dy);
    void zoom(float delta);
    void pan(float dx, float dy);
    void reset();
};
```

- [ ] **Step 2: Write camera.cpp**

```cpp
#include "camera.h"
#include <cmath>
#include <algorithm>

glm::vec3 OrbitCamera::getPosition() const {
    glm::vec3 pos;
    pos.x = target.x + distance * cosf(pitch) * sinf(yaw);
    pos.y = target.y + distance * sinf(pitch);
    pos.z = target.z + distance * cosf(pitch) * cosf(yaw);
    return pos + panOffset;
}

glm::mat4 OrbitCamera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target + panOffset, glm::vec3(0.0f, 1.0f, 0.0f));
}

void OrbitCamera::rotate(float dx, float dy) {
    yaw   += dx * rotateSensitivity;
    pitch += dy * rotateSensitivity;
    pitch = std::clamp(pitch, minPitch, maxPitch);
}

void OrbitCamera::zoom(float delta) {
    distance -= delta * zoomSensitivity;
    distance = std::clamp(distance, minDistance, maxDistance);
}

void OrbitCamera::pan(float dx, float dy) {
    // Pan in the camera's local right and up directions
    glm::vec3 forward = glm::normalize(target + panOffset - getPosition());
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::cross(right, forward);

    panOffset += right * (-dx * panSensitivity) + up * (dy * panSensitivity);
}

void OrbitCamera::reset() {
    target = glm::vec3(0.0f);
    distance = 50.0f;
    yaw = 0.0f;
    pitch = 0.6f;
    panOffset = glm::vec3(0.0f);
}
```

- [ ] **Step 3: Wire camera into main.cpp**

Add these globals and callbacks to `main.cpp`, and update the render loop to use the camera's view/projection matrices.

Add below the includes:
```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

OrbitCamera camera;
bool leftMouseDown = false;
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
```

Add these callback functions before `main()`:
```cpp
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        leftMouseDown = (action == GLFW_PRESS);
        if (leftMouseDown) glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rightMouseDown = (action == GLFW_PRESS);
        if (rightMouseDown) glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    double dx = xpos - lastMouseX;
    double dy = ypos - lastMouseY;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (leftMouseDown)  camera.rotate((float)dx, (float)-dy);
    if (rightMouseDown) camera.pan((float)dx, (float)dy);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom((float)yoffset);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_R) camera.reset();
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
}
```

Register callbacks in `main()` after `glfwSetFramebufferSizeCallback`:
```cpp
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
```

- [ ] **Step 4: Build and run**

```bash
cd build && cmake --build . --config Debug
cd ../bin && ./final_project_d.exe
```

Expected: same dark window, but no crashes. Camera state is being updated (no visual proof yet — grid will use it).

- [ ] **Step 5: Commit**

```bash
git add src/camera.h src/camera.cpp src/main.cpp
git commit -m "Add orbit camera with mouse rotate, zoom, and pan"
```

---

### Task 5: Grid Mesh with Instanced Rendering

**Files:**
- Create: `src/grid.h`
- Create: `src/grid.cpp`
- Create: `bin/media/shaders/grid/grid.vert`
- Create: `bin/media/shaders/grid/grid.frag`

- [ ] **Step 1: Write grid.h**

```cpp
#pragma once
#include <GL3/gl3w.h>
#include <glm/glm.hpp>
#include <vector>

struct GridConfig {
    int width      = 64;   // columns
    int height     = 48;   // rows
    float cellSize     = 1.0f;  // XZ footprint per column
    float maxHeight    = 5.0f;  // max extrusion height
    float sensitivity  = 1.0f;  // motion sensitivity multiplier
};

struct ColumnInstance {
    glm::vec2 gridPos;   // (col, row) world position
    glm::vec3 color;     // RGB from webcam
    float height;        // current extrusion height
};

class Grid {
public:
    void init(const GridConfig& config);
    void updateFromFrame(const unsigned char* rgbData, int frameWidth, int frameHeight);
    void render() const;
    void cleanup();

    const GridConfig& getConfig() const { return config_; }
    int instanceCount() const { return config_.width * config_.height; }

private:
    GridConfig config_;
    std::vector<ColumnInstance> instances_;

    GLuint boxVAO_ = 0;
    GLuint boxVBO_ = 0;
    GLuint boxEBO_ = 0;
    GLuint instanceVBO_ = 0;

    void createBoxMesh();
    void createInstanceBuffer();
};
```

- [ ] **Step 2: Write grid.cpp**

```cpp
#include "grid.h"

// Unit box: 8 vertices, 36 indices (12 triangles)
// Box spans [0,1] in X and Z, [0,1] in Y (base at Y=0, top at Y=1)
// Normals point outward from each face

struct BoxVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

static const BoxVertex BOX_VERTICES[] = {
    // Front face (z = 1)
    {{0, 0, 1}, {0, 0, 1}}, {{1, 0, 1}, {0, 0, 1}},
    {{1, 1, 1}, {0, 0, 1}}, {{0, 1, 1}, {0, 0, 1}},
    // Back face (z = 0)
    {{1, 0, 0}, {0, 0, -1}}, {{0, 0, 0}, {0, 0, -1}},
    {{0, 1, 0}, {0, 0, -1}}, {{1, 1, 0}, {0, 0, -1}},
    // Left face (x = 0)
    {{0, 0, 0}, {-1, 0, 0}}, {{0, 0, 1}, {-1, 0, 0}},
    {{0, 1, 1}, {-1, 0, 0}}, {{0, 1, 0}, {-1, 0, 0}},
    // Right face (x = 1)
    {{1, 0, 1}, {1, 0, 0}}, {{1, 0, 0}, {1, 0, 0}},
    {{1, 1, 0}, {1, 0, 0}}, {{1, 1, 1}, {1, 0, 0}},
    // Top face (y = 1)
    {{0, 1, 1}, {0, 1, 0}}, {{1, 1, 1}, {0, 1, 0}},
    {{1, 1, 0}, {0, 1, 0}}, {{0, 1, 0}, {0, 1, 0}},
    // Bottom face (y = 0)
    {{0, 0, 0}, {0, -1, 0}}, {{1, 0, 0}, {0, -1, 0}},
    {{1, 0, 1}, {0, -1, 0}}, {{0, 0, 1}, {0, -1, 0}},
};

static const GLuint BOX_INDICES[] = {
     0,  1,  2,   2,  3,  0,   // front
     4,  5,  6,   6,  7,  4,   // back
     8,  9, 10,  10, 11,  8,   // left
    12, 13, 14,  14, 15, 12,   // right
    16, 17, 18,  18, 19, 16,   // top
    20, 21, 22,  22, 23, 20,   // bottom
};

void Grid::init(const GridConfig& config) {
    config_ = config;
    instances_.resize(instanceCount());

    // Initialize instance positions and defaults
    for (int row = 0; row < config_.height; ++row) {
        for (int col = 0; col < config_.width; ++col) {
            int i = row * config_.width + col;
            instances_[i].gridPos = glm::vec2(
                col * config_.cellSize,
                row * config_.cellSize
            );
            instances_[i].color = glm::vec3(0.2f);
            instances_[i].height = 0.1f;
        }
    }

    createBoxMesh();
    createInstanceBuffer();
}

void Grid::createBoxMesh() {
    glGenVertexArrays(1, &boxVAO_);
    glGenBuffers(1, &boxVBO_);
    glGenBuffers(1, &boxEBO_);

    glBindVertexArray(boxVAO_);

    // Box vertex data
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BOX_VERTICES), BOX_VERTICES, GL_STATIC_DRAW);

    // Position: location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoxVertex), (void*)0);

    // Normal: location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BoxVertex),
        (void*)offsetof(BoxVertex, normal));

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(BOX_INDICES), BOX_INDICES, GL_STATIC_DRAW);
}

void Grid::createInstanceBuffer() {
    glGenBuffers(1, &instanceVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER,
        instances_.size() * sizeof(ColumnInstance),
        instances_.data(), GL_DYNAMIC_DRAW);

    // gridPos: location 2 (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, gridPos));
    glVertexAttribDivisor(2, 1);

    // color: location 3 (vec3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, color));
    glVertexAttribDivisor(3, 1);

    // height: location 4 (float)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, height));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
}

void Grid::updateFromFrame(const unsigned char* rgbData, int frameWidth, int frameHeight) {
    // rgbData is expected to be RGB, 3 bytes per pixel, sized to config_.width x config_.height
    // If frameWidth/frameHeight don't match config, caller should have resized first
    for (int row = 0; row < config_.height; ++row) {
        for (int col = 0; col < config_.width; ++col) {
            int i = row * config_.width + col;
            int pixelIdx = (row * frameWidth + col) * 3;
            instances_[i].color = glm::vec3(
                rgbData[pixelIdx + 0] / 255.0f,
                rgbData[pixelIdx + 1] / 255.0f,
                rgbData[pixelIdx + 2] / 255.0f
            );
        }
    }

    // Upload updated instance data (buffer orphaning)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER,
        instances_.size() * sizeof(ColumnInstance),
        nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        instances_.size() * sizeof(ColumnInstance),
        instances_.data());
}

void Grid::render() const {
    glBindVertexArray(boxVAO_);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, instanceCount());
    glBindVertexArray(0);
}

void Grid::cleanup() {
    glDeleteVertexArrays(1, &boxVAO_);
    glDeleteBuffers(1, &boxVBO_);
    glDeleteBuffers(1, &boxEBO_);
    glDeleteBuffers(1, &instanceVBO_);
}
```

- [ ] **Step 3: Write grid.vert**

Create `bin/media/shaders/grid/grid.vert`:

```glsl
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aGridPos;
layout(location = 3) in vec3 aColor;
layout(location = 4) in float aHeight;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main() {
    // Scale Y by height, keep X and Z at cell size
    vec3 scaled = aPos;
    scaled.y *= max(aHeight, 0.05);

    // Translate to grid position (gridPos.x -> world X, gridPos.y -> world Z)
    vec3 worldPos = scaled + vec3(aGridPos.x, 0.0, aGridPos.y);

    FragPos = worldPos;
    Normal = aNormal;
    Color = aColor;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
```

- [ ] **Step 4: Write grid.frag**

Create `bin/media/shaders/grid/grid.frag`:

```glsl
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

out vec4 FragColor;

void main() {
    // Simple directional light for now (Phong added in a later task)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 result = Color * (0.3 + 0.7 * diff);
    FragColor = vec4(result, 1.0);
}
```

- [ ] **Step 5: Wire grid and shaders into main.cpp**

Add to `main()`, after `glEnable(GL_DEPTH_TEST)`:

```cpp
    // Shaders
    GLuint shaderProgram = loadShaderProgram(
        "media/shaders/grid/grid.vert",
        "media/shaders/grid/grid.frag"
    );
    if (!shaderProgram) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }

    // Grid
    GridConfig gridConfig;
    Grid grid;
    grid.init(gridConfig);

    // Center camera on the grid
    camera.target = glm::vec3(
        gridConfig.width * gridConfig.cellSize * 0.5f,
        0.0f,
        gridConfig.height * gridConfig.cellSize * 0.5f
    );
```

Replace the render loop body with:

```cpp
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View and projection matrices
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = (float)fbWidth / (float)fbHeight;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 500.0f);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

        grid.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    grid.cleanup();
    glDeleteProgram(shaderProgram);
```

- [ ] **Step 6: Build and run**

```bash
cd build && cmake --build . --config Debug
cd ../bin && ./final_project_d.exe
```

Expected: a grid of small dark gray boxes visible in the window. You can orbit with left-drag, zoom with scroll, pan with right-drag. Press R to reset the camera.

- [ ] **Step 7: Commit**

```bash
git add src/grid.h src/grid.cpp src/main.cpp bin/media/shaders/grid/grid.vert bin/media/shaders/grid/grid.frag
git commit -m "Add instanced grid mesh with orbit camera and basic shading"
```

---

### Task 6: Webcam Capture + Grid Color Update

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add OpenCV includes and webcam capture to main.cpp**

Add to includes at top of `main.cpp`:
```cpp
#include <opencv2/opencv.hpp>
```

Add after grid initialization (before the render loop):
```cpp
    // Webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open webcam" << std::endl;
        return -1;
    }

    cv::Mat frame, small, rgb;
```

- [ ] **Step 2: Add frame capture and grid update to the render loop**

Add at the top of the render loop, before the `glClear` call:

```cpp
        // Capture webcam frame and update grid colors
        cap >> frame;
        if (!frame.empty()) {
            cv::resize(frame, small, cv::Size(gridConfig.width, gridConfig.height));
            cv::cvtColor(small, rgb, cv::COLOR_BGR2RGB);
            grid.updateFromFrame(rgb.data, gridConfig.width, gridConfig.height);
        }
```

Add cleanup before `glfwTerminate()`:
```cpp
    cap.release();
```

- [ ] **Step 3: Build and run**

```bash
cd build && cmake --build . --config Debug
cd ../bin && ./final_project_d.exe
```

Expected: the grid of boxes now shows live webcam colors. Each box takes the color of its corresponding webcam pixel. You can orbit/zoom/pan around the colored grid.

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "Add webcam capture and live grid color updates"
```

---

### Task 7: Final Verification

- [ ] **Step 1: Full clean build**

```bash
cd build && cmake .. -DOpenCV_DIR=C:/opencv/build && cmake --build . --config Debug
```

Expected: no warnings, no errors.

- [ ] **Step 2: Run and verify all features**

```bash
cd ../bin && ./final_project_d.exe
```

Verify:
1. Window opens with the title "Webcam Pixel Grid"
2. Grid of colored boxes visible, colors coming from webcam
3. Left-drag orbits the camera
4. Scroll zooms in/out
5. Right-drag pans
6. R key resets camera
7. ESC closes the window
8. No crashes on close

- [ ] **Step 3: Commit any fixes if needed**
