#version 330 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uLightSpaceMatrix;

void main()
{
    vec4 worldPos = uModelMatrix * vec4(aVertexPosition, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aVertexTexCoords;
    
    //mat3 normalMatrix = transpose(inverse(mat3(uModelMatrix)));
    //Normal = normalMatrix * aVertexNormal;
    Normal = normalize(vec3(uModelMatrix * vec4(aVertexNormal, 0.0)));

    gl_Position = uProjectionMatrix * worldPos;
}