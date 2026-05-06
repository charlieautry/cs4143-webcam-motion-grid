// orbit camera controlled by the mouse
// uses spherical coords yaw pitch distance around a target point

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct OrbitCamera {
    glm::vec3 target = glm::vec3(0.0f); // point the cam orbits around
    float distance = 50.0f; // distance from target
    float yaw = 0.0f; // horizontal angle in radians
    float pitch = 0.6f; // vertical angle in radians ~34 deg
    glm::vec3 panOffset = glm::vec3(0.0f);

    // input sensitivity
    float rotateSensitivity = 0.005f;
    float zoomSensitivity = 2.0f;
    float panSensitivity = 0.05f;

    // pitch clamp so the cam doesnt flip
    float minPitch = -1.5f; // ~ -86 deg
    float maxPitch = 1.5f; // ~ 86 deg

    // zoom clamp
    float minDistance = 5.0f;
    float maxDistance = 200.0f;

    glm::vec3 getPosition() const;
    glm::mat4 getViewMatrix() const;

    void rotate(float dx, float dy); // left click drag
    void zoom(float delta); // scroll wheel
    void pan(float dx, float dy); // right click drag
    void reset(); // back to defaults
};
