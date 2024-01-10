#version 430 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;

uniform mat4 lightMatrixTest;
uniform mat4 modelMat;

void main()
{
    gl_Position = lightMatrixTest * modelMat * vec4(vertexPosition, 1.0);
}