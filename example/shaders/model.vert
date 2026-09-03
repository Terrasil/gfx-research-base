#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

uniform mat4 uMvp;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vPosition;
out vec2 vUv;

void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vPosition = world.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vUv = aUv;
    gl_Position = uMvp * vec4(aPosition, 1.0);
}
