#include "grid.h"

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
