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
