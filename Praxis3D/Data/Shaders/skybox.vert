#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec2 textureCoord;

out vec2 texCoord;

uniform mat4 MVP;

void main(void) 
{
	texCoord = textureCoord;
	
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}