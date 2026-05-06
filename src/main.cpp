// main entry point webcam feeds the grid we draw it w/ phong + a hud on top

#include <GL3/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <opencv2/opencv.hpp>

#include "shader.h"
#include "camera.h"
#include "grid.h"
#include "text.h"

// window
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// global state used by glfw callbacks
OrbitCamera camera;
Grid* g_grid = nullptr;
bool leftMouseDown = false;
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

// visual mode toggles
bool monochrome = false;
bool mirrorMode = false;
bool showHUD = true;

// fps stuff
double lastTime = 0.0;
int frameCount = 0;
float fps = 0.0f;

// grid sizes you cycle through with [ and ]
struct GridPreset { int w, h; const char* label; };
static const GridPreset GRID_PRESETS[] = {
    { 32,  24, "32x24"   },
    { 48,  36, "48x36"   },
    { 64,  48, "64x48"   },
    { 96,  72, "96x72"   },
    {128,  96, "128x96"  },
};
static const int NUM_PRESETS = sizeof(GRID_PRESETS) / sizeof(GRID_PRESETS[0]);
int currentPreset = 2; // default 64x48

// glfw callbacks

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

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

// center the cam on whatever grid is current
static void centerCameraOnGrid() {
    if (!g_grid) return;
    const GridConfig& cfg = g_grid->getConfig();
    camera.target = glm::vec3(
        cfg.width * cfg.cellSize * 0.5f,
        0.0f,
        cfg.height * cfg.cellSize * 0.5f
    );
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // r resets the cam to default and centers on the grid
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camera.reset();
        centerCameraOnGrid();
    }

    // f focus center and a 45 degree view angle
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        centerCameraOnGrid();
        camera.panOffset = glm::vec3(0.0f);
        camera.yaw = glm::radians(45.0f);
        camera.pitch = glm::radians(45.0f);
        camera.distance = 50.0f;
    }

    // visual mode toggles press only no repeat
    if (key == GLFW_KEY_C && action == GLFW_PRESS) monochrome = !monochrome;
    if (key == GLFW_KEY_M && action == GLFW_PRESS) mirrorMode = !mirrorMode;
    if (key == GLFW_KEY_H && action == GLFW_PRESS) showHUD = !showHUD;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);

    // grid params adjustable while running supports key repeat
    if (g_grid) {
        GridConfig& cfg = g_grid->getConfig();
        if (key == GLFW_KEY_UP)    cfg.heightMultiplier = std::min(cfg.heightMultiplier * 2.0f, 8.0f);
        if (key == GLFW_KEY_DOWN)  cfg.heightMultiplier = std::max(cfg.heightMultiplier / 2.0f, 0.25f);
        if (key == GLFW_KEY_RIGHT) cfg.sensitivity = std::min(cfg.sensitivity + 0.05f, 1.0f);
        if (key == GLFW_KEY_LEFT)  cfg.sensitivity = std::max(cfg.sensitivity - 0.05f, 0.0f);

        // grid res presets flow res stays fixed only the grid changes
        if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS && currentPreset > 0) {
            currentPreset--;
            g_grid->resize(GRID_PRESETS[currentPreset].w, GRID_PRESETS[currentPreset].h);
        }
        if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS && currentPreset < NUM_PRESETS - 1) {
            currentPreset++;
            g_grid->resize(GRID_PRESETS[currentPreset].w, GRID_PRESETS[currentPreset].h);
        }
    }
}

// main

int main() {
    // init glfw and open an opengl 3.3 core profile window
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

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

    if (gl3wInit()) {
        std::cerr << "Failed to initialize gl3w" << std::endl;
        return -1;
    }

    // register input callbacks
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glEnable(GL_DEPTH_TEST);

    // load grid shaders w/ phong lighting per instance color and height
    GLuint shaderProgram = loadShaderProgram(
        "media/shaders/grid/grid.vert",
        "media/shaders/grid/grid.frag"
    );
    if (!shaderProgram) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }

    // cache uniform locations
    GLint locView = glGetUniformLocation(shaderProgram, "view");
    GLint locProj = glGetUniformLocation(shaderProgram, "projection");
    GLint locLightPos = glGetUniformLocation(shaderProgram, "lightPos");
    GLint locViewPos = glGetUniformLocation(shaderProgram, "viewPos");
    GLint locLightIntensity = glGetUniformLocation(shaderProgram, "lightIntensity");

    // build the instanced column grid
    Grid grid;
    grid.init(GridConfig{});
    g_grid = &grid;
    centerCameraOnGrid();

    // bitmap text for the hud overlay
    TextRenderer textRenderer;
    textRenderer.init();

    // grab the default webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open webcam" << std::endl;
        return -1;
    }

    cv::Mat frame, small, rgb, gray;
    // optical flow runs at higher res lower was a mess late one night
    const int FLOW_W = 320, FLOW_H = 240;
    cv::Mat flowFrame, flowGray, flowBlur, prevFlowGray, flow, flowMag, motion;

    // render loop
    while (!glfwWindowShouldClose(window)) {

        // 1 webcam capture grid color + motion update
        cap >> frame;
        if (!frame.empty()) {
            const GridConfig& cfg = grid.getConfig();
            if (mirrorMode) cv::flip(frame, frame, 1);
            cv::resize(frame, small, cv::Size(cfg.width, cfg.height));
            cv::cvtColor(small, gray, cv::COLOR_BGR2GRAY);

            if (monochrome) {
                cv::Mat grayRGB;
                cv::cvtColor(gray, grayRGB, cv::COLOR_GRAY2RGB);
                grid.updateFromFrame(grayRGB.data);
            } else {
                cv::cvtColor(small, rgb, cv::COLOR_BGR2RGB);
                grid.updateFromFrame(rgb.data);
            }

            // motion detection dense optical flow farneback
            // runs at 320x240 then gets downsampled to grid res
            cv::resize(frame, flowFrame, cv::Size(FLOW_W, FLOW_H));
            cv::cvtColor(flowFrame, flowGray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(flowGray, flowBlur, cv::Size(3, 3), 0);

            if (!prevFlowGray.empty()) {
                cv::calcOpticalFlowFarneback(prevFlowGray, flowBlur, flow,
                    0.5, 3, 15, 3, 5, 1.2, 0);

                // split flow into x/y channels and grab the magnitude simd is doing the work here
                cv::Mat flowChannels[2], mag, angle;
                cv::split(flow, flowChannels);
                cv::cartToPolar(flowChannels[0], flowChannels[1], mag, angle, false);

                // normalize clamp at 4 pixels of movement to 255
                mag.convertTo(flowMag, CV_8UC1, 255.0 / 4.0);

                // downsample to grid res inter_area keeps the motion energy alive
                cv::resize(flowMag, motion, cv::Size(cfg.width, cfg.height), 0, 0, cv::INTER_AREA);
                grid.updateMotion(motion.data);
            } else {
                // first frame just push colors to the gpu with zero motion
                motion = cv::Mat::zeros(cfg.height, cfg.width, CV_8UC1);
                grid.updateMotion(motion.data);
            }
            flowBlur.copyTo(prevFlowGray);
        }

        // 2 clear and set up matrices
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = (float)fbWidth / (float)fbHeight;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 500.0f);

        // 3 draw the grid with phong shading
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(locView, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, &projection[0][0]);

        // dynamic lighting intensity scales with average motion across the grid
        const GridConfig& cfg = grid.getConfig();
        glm::vec3 gridCenter(cfg.width * cfg.cellSize * 0.5f, 0.0f, cfg.height * cfg.cellSize * 0.5f);
        glm::vec3 lightPos = gridCenter + glm::vec3(0.0f, 40.0f, 20.0f);
        glm::vec3 viewPos = camera.getPosition();
        float lightIntensity = 0.6f + grid.getAverageMotion() * 2.0f;

        glUniform3fv(locLightPos, 1, &lightPos[0]);
        glUniform3fv(locViewPos, 1, &viewPos[0]);
        glUniform1f(locLightIntensity, lightIntensity);

        grid.render();

        // 4 fps counter updates once a second
        frameCount++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            fps = (float)frameCount / (float)(currentTime - lastTime);
            frameCount = 0;
            lastTime = currentTime;
        }

        // 5 hud overlay
        if (showHUD) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(0);
            oss << "FPS: " << fps;
            textRenderer.renderText(oss.str(), 10, 10, 2.0f, fbWidth, fbHeight);

            oss.str("");
            oss << "Mode: " << (monochrome ? "Mono" : "Color");
            if (mirrorMode) oss << " [Mirror]";
            textRenderer.renderText(oss.str(), 10, 30, 2.0f, fbWidth, fbHeight);

            oss.str("");
            oss << "Height: ";
            if (cfg.heightMultiplier >= 1.0f)
                oss << std::setprecision(0) << cfg.heightMultiplier << "x";
            else
                oss << std::setprecision(2) << cfg.heightMultiplier << "x";
            oss << "  Sens: " << std::setprecision(0) << (cfg.sensitivity * 100.0f) << "%";
            textRenderer.renderText(oss.str(), 10, 50, 2.0f, fbWidth, fbHeight);

            oss.str("");
            oss << "Grid: " << GRID_PRESETS[currentPreset].label;
            textRenderer.renderText(oss.str(), 10, 70, 2.0f, fbWidth, fbHeight);

            textRenderer.renderText("H:HUD C:Color M:Mirror R:Reset F:Focus [/]:Grid", 10, fbHeight - 20.0f, 1.5f, fbWidth, fbHeight);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    cap.release();
    textRenderer.cleanup();
    grid.cleanup();
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
