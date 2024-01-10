#version 430 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMat;
uniform mat4 MVP;

void main(void) 
{
	gl_Position = modelMat * vec4(vertexPosition, 1.0);
}