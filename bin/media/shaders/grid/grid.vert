// vertex shader for the instanced column grid
// each instance gets a grid pos webcam color and a motion driven height
// unit box gets scaled on y by height then translated to its grid cell

#version 330 core
layout(location = 0) in vec3 aPos; // unit box vert position
layout(location = 1) in vec3 aNormal; // unit box face normal
layout(location = 2) in vec2 aGridPos; // per instance col world pos xz
layout(location = 3) in vec3 aColor; // per instance webcam rgb
layout(location = 4) in float aHeight; // per instance extrusion height

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main() {
    // scale y by height min 0.05 so cols are always visible
    vec3 scaled = aPos;
    scaled.y *= max(aHeight, 0.05);

    // translate to grid position gridPos.x to world x gridPos.y to world z
    vec3 worldPos = scaled + vec3(aGridPos.x, 0.0, aGridPos.y);

    FragPos = worldPos;
    Normal = aNormal; // no model transform needed model is always identity
    Color = aColor;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
