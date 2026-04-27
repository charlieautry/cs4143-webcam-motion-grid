#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aGridPos;
layout(location = 3) in vec3 aColor;
layout(location = 4) in float aHeight;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main() {
    // Scale Y by height, keep X and Z at cell size
    vec3 scaled = aPos;
    scaled.y *= max(aHeight, 0.05);

    // Translate to grid position (gridPos.x -> world X, gridPos.y -> world Z)
    vec3 worldPos = scaled + vec3(aGridPos.x, 0.0, aGridPos.y);

    FragPos = worldPos;
    Normal = aNormal;
    Color = aColor;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
