#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

out vec4 FragColor;

void main() {
    // Simple directional light for now (Phong added in a later task)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 result = Color * (0.3 + 0.7 * diff);
    FragColor = vec4(result, 1.0);
}
