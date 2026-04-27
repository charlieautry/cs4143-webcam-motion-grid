#include "camera.h"
#include <cmath>

static float clampf(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

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
    pitch = clampf(pitch, minPitch, maxPitch);
}

void OrbitCamera::zoom(float delta) {
    distance -= delta * zoomSensitivity;
    distance = clampf(distance, minDistance, maxDistance);
}

void OrbitCamera::pan(float dx, float dy) {
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
