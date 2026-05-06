// fragment shader phong shading ambient + diffuse + specular
// light intensity scales with how much motion is in the scene
// more movement brighter lighting still scene dimmer ambient vibe

#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 lightPos; // world space light position above grid center
uniform vec3 viewPos; // cam position for specular calc
uniform float lightIntensity; // dynamic scales with avg motion

out vec4 FragColor;

void main() {
    // ambient baseline so cols are always visible
    vec3 ambient = 0.15 * Color;

    // diffuse lambertian shading from the point light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Color * lightIntensity;

    // specular phong reflection shininess is 32
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(0.3) * lightIntensity;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
