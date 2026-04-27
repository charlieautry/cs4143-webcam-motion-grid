#pragma once
#include <GL3/gl3w.h>

// Loads vertex and fragment shader files, compiles and links them.
// Returns the program ID, or 0 on failure (errors printed to stderr).
GLuint loadShaderProgram(const char* vertexPath, const char* fragmentPath);
