#version 460 core
in vec3 vNormal;
in vec3 vPosition;
in vec2 vUv;

uniform vec3 uEye;
out vec4 oColor;

void main() {
    vec3 n = normalize(vNormal);
    vec3 l = normalize(vec3(0.4, 1.0, 0.55));
    vec3 v = normalize(uEye - vPosition);
    vec3 h = normalize(l + v);
    float diffuse = max(dot(n, l), 0.0);
    float specular = pow(max(dot(n, h), 0.0), 48.0);
    vec3 base = vec3(0.64, 0.70, 0.80);
    oColor = vec4(base * (0.15 + 0.85 * diffuse) + specular * 0.2, 1.0);
}
