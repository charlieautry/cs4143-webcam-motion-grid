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
