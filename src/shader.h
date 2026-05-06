#pragma once
#include <GL3/gl3w.h>

// loads vert + frag shader files compiles and links them
// returns the program id or 0 on failure errors go to stderr
GLuint loadShaderProgram(const char* vertexPath, const char* fragmentPath);
