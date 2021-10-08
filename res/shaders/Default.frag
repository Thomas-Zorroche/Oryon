#version 330 core

out vec4 fFragColor;


void main()
{
    vec3 finalColor = vec3(1.0, 0.0, 0.0);
    fFragColor = vec4(finalColor, 1.0f);
}


