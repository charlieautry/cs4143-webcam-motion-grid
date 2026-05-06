// instanced column grid mesh gen webcam color mapping
// motion driven height extrusion w/ smooth lerp

#include "grid.h"

// unit box mesh 24 verts w/ per face normals 36 indices

struct BoxVertex {
    glm::vec3 position;
    glm::vec3 normal;
};

static const BoxVertex BOX_VERTICES[] = {
    // front face z = 1
    {{0, 0, 1}, {0, 0, 1}}, {{1, 0, 1}, {0, 0, 1}},
    {{1, 1, 1}, {0, 0, 1}}, {{0, 1, 1}, {0, 0, 1}},
    // back face z = 0
    {{1, 0, 0}, {0, 0, -1}}, {{0, 0, 0}, {0, 0, -1}},
    {{0, 1, 0}, {0, 0, -1}}, {{1, 1, 0}, {0, 0, -1}},
    // left face x = 0
    {{0, 0, 0}, {-1, 0, 0}}, {{0, 0, 1}, {-1, 0, 0}},
    {{0, 1, 1}, {-1, 0, 0}}, {{0, 1, 0}, {-1, 0, 0}},
    // right face x = 1
    {{1, 0, 1}, {1, 0, 0}}, {{1, 0, 0}, {1, 0, 0}},
    {{1, 1, 0}, {1, 0, 0}}, {{1, 1, 1}, {1, 0, 0}},
    // top face y = 1
    {{0, 1, 1}, {0, 1, 0}}, {{1, 1, 1}, {0, 1, 0}},
    {{1, 1, 0}, {0, 1, 0}}, {{0, 1, 0}, {0, 1, 0}},
    // bottom face y = 0
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

// init

void Grid::init(const GridConfig& config) {
    config_ = config;
    int count = instanceCount();
    instances_.resize(count);
    currentHeights_.resize(count, 0.1f);

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

void Grid::resize(int newWidth, int newHeight) {
    config_.width = newWidth;
    config_.height = newHeight;
    int count = instanceCount();
    instances_.resize(count);
    currentHeights_.assign(count, 0.1f);
    averageMotion_ = 0.0f;

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

    // reallocate the instance buffer for the new count
    glBindVertexArray(boxVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER,
        instances_.size() * sizeof(ColumnInstance),
        instances_.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(0);
}

// gpu resource setup

void Grid::createBoxMesh() {
    glGenVertexArrays(1, &boxVAO_);
    glGenBuffers(1, &boxVBO_);
    glGenBuffers(1, &boxEBO_);

    glBindVertexArray(boxVAO_);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BOX_VERTICES), BOX_VERTICES, GL_STATIC_DRAW);

    // vertex attribs position loc 0 and normal loc 1
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoxVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BoxVertex),
        (void*)offsetof(BoxVertex, normal));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(BOX_INDICES), BOX_INDICES, GL_STATIC_DRAW);
}

void Grid::createInstanceBuffer() {
    glGenBuffers(1, &instanceVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER,
        instances_.size() * sizeof(ColumnInstance),
        instances_.data(), GL_DYNAMIC_DRAW);

    // per instance attribs divisor 1
    // gridPos loc 2 color loc 3 height loc 4
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, gridPos));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, color));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ColumnInstance),
        (void*)offsetof(ColumnInstance, height));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
}

// per frame updates

void Grid::updateFromFrame(const unsigned char* rgbData) {
    // map webcam pixels rgb 0-255 to per instance color floats 0-1
    for (int row = 0; row < config_.height; ++row) {
        for (int col = 0; col < config_.width; ++col) {
            int i = row * config_.width + col;
            int pixelIdx = (row * config_.width + col) * 3;
            instances_[i].color = glm::vec3(
                rgbData[pixelIdx + 0] / 255.0f,
                rgbData[pixelIdx + 1] / 255.0f,
                rgbData[pixelIdx + 2] / 255.0f
            );
        }
    }
}

void Grid::updateMotion(const unsigned char* motionData) {
    int count = instanceCount();

    // sensitivity is a noise gate motion below threshold gets ignored
    float threshold = (1.0f - config_.sensitivity) * 0.3f;
    float motionSum = 0.0f;
    for (int i = 0; i < count; ++i) {
        float motionValue = motionData[i] / 255.0f;

        // gate ditches motion below threshold and remaps the rest to 0-1
        motionValue = (motionValue > threshold)
            ? (motionValue - threshold) / (1.0f - threshold)
            : 0.0f;

        // asymmetric lerp fast rise slow fall the trail looked off until this
        float baseHeight = 15.0f;
        float target = motionValue * baseHeight * config_.heightMultiplier;
        float lerpRate = (target > currentHeights_[i]) ? config_.riseSpeed : config_.fallSpeed;
        currentHeights_[i] += (target - currentHeights_[i]) * lerpRate;

        float minHeight = 0.1f;
        instances_[i].height = (currentHeights_[i] > minHeight) ? currentHeights_[i] : minHeight;

        motionSum += motionValue;
    }

    averageMotion_ = motionSum / (float)count;

    // push instance data to gpu buffer orphaning so async updates dont blow up
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER,
        instances_.size() * sizeof(ColumnInstance),
        nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        instances_.size() * sizeof(ColumnInstance),
        instances_.data());
}

// rendering

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
