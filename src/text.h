#pragma once
#include <GL3/gl3w.h>
#include <string>

class TextRenderer {
public:
    void init();
    void renderText(const std::string& text, float x, float y,
                    float scale, int screenWidth, int screenHeight);
    void cleanup();

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint shaderProgram_ = 0;
    GLuint fontTexture_ = 0;

    void createFontTexture();
    void createShader();
};
