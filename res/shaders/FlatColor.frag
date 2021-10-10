#version 330 core

out vec4 fFragColor;

uniform vec3 uColor;

void main()
{
    fFragColor = vec4(uColor, 1.0);
}


