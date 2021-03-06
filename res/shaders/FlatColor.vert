#version 330 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uLightSpaceMatrix;

// Outputs
out vec3 vNormal;
out vec3 vFragPos;
out vec4 vFragPosLightSpace;
out vec2 vTexCoords;

void main() {
    vec4 vertexPosition = vec4(aVertexPosition, 1);

    vFragPos = vec3(uModelMatrix * vertexPosition);

    vFragPosLightSpace = uLightSpaceMatrix * vec4(vFragPos, 1.0);

    vNormal = normalize(vec3(uModelMatrix * vec4(aVertexNormal, 0.0)));

    vTexCoords = aVertexTexCoords;

    gl_Position =  uProjectionMatrix * uModelMatrix * vertexPosition;
}