#version 430 core

layout(location = 0) in vec3 vertexPosition;
//layout(location = 2) in vec2 textureCoord;

out vec3 cubemapCoord;

uniform mat4 MVP;

void main(void) 
{	
	cubemapCoord = vertexPosition;
	
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}