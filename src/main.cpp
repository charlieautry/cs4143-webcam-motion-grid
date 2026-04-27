#include <GL3/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "shader.h"
#include "camera.h"
#include "grid.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

OrbitCamera camera;
bool leftMouseDown = false;
bool rightMouseDown = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_R) camera.reset();
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
}

int main() {
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

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glEnable(GL_DEPTH_TEST);

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

    // Webcam
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open webcam" << std::endl;
        return -1;
    }

    cv::Mat frame, small, rgb;

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Capture webcam frame and update grid colors
        cap >> frame;
        if (!frame.empty()) {
            cv::resize(frame, small, cv::Size(gridConfig.width, gridConfig.height));
            cv::cvtColor(small, rgb, cv::COLOR_BGR2RGB);
            grid.updateFromFrame(rgb.data, gridConfig.width, gridConfig.height);
        }

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

    cap.release();
    grid.cleanup();
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
