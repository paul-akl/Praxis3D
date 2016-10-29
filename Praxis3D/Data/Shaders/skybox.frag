#version 330

layout(location = 1) out vec4 finalBuffer;

in vec2 texCoord;

uniform sampler2D diffuseTexture;

void main(void) 
{
	// Write the color from model's texture
	finalBuffer	= texture(diffuseTexture, texCoord).rgba;
}