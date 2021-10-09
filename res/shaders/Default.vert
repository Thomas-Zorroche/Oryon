#version 330 core

layout(location = 0) in vec3 aVertexPosition;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;

void main() {
    gl_Position = uModelMatrix * uProjectionMatrix * vec4(aVertexPosition, 1);
}