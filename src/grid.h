// instanced column grid driven by webcam color + motion
// every cell is a rectangular prism color comes from the webcam height comes from motion detection

#pragma once
#include <GL3/gl3w.h>
#include <glm/glm.hpp>
#include <vector>

// grid params everything else reads off this
struct GridConfig {
    int width = 64; // cols
    int height = 48; // rows
    float cellSize = 1.0f; // xz footprint per col
    float heightMultiplier = 1.0f; // height scale 0.25x to 8x
    float sensitivity = 0.7f; // noise gate 0 = only big motion 1 = all motion
    float riseSpeed = 0.25f; // lerp rate going up
    float fallSpeed = 0.12f; // lerp rate coming down
};

// per instance data we shove to the gpu every frame
struct ColumnInstance {
    glm::vec2 gridPos; // col row world position
    glm::vec3 color; // rgb from webcam
    float height; // current extrusion height smoothed
};

class Grid {
public:
    void init(const GridConfig& config);
    void resize(int newWidth, int newHeight);
    void updateFromFrame(const unsigned char* rgbData);
    void updateMotion(const unsigned char* motionData);
    void render() const;
    void cleanup();

    GridConfig& getConfig() { return config_; }
    const GridConfig& getConfig() const { return config_; }
    int instanceCount() const { return config_.width * config_.height; }
    float getAverageMotion() const { return averageMotion_; }

private:
    GridConfig config_;
    std::vector<ColumnInstance> instances_;
    std::vector<float> currentHeights_; // smoothed heights lerp state
    float averageMotion_ = 0.0f; // mean motion across all cells 0-1

    GLuint boxVAO_ = 0;
    GLuint boxVBO_ = 0;
    GLuint boxEBO_ = 0;
    GLuint instanceVBO_ = 0;

    void createBoxMesh();
    void createInstanceBuffer();
};
