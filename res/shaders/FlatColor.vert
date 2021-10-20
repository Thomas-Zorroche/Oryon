#version 330 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;

// Outputs
out vec3 vNormal;
out vec3 vFragPos;

void main() {
    vec4 vertexPosition = vec4(aVertexPosition, 1);

    vFragPos = vec3(uModelMatrix * vertexPosition);

    vNormal = normalize(vec3(uModelMatrix * vec4(aVertexNormal, 0.0)));

    gl_Position =  uProjectionMatrix * uModelMatrix * vertexPosition;
}