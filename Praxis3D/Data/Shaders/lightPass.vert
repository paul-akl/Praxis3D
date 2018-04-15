#version 430 core

#define ENABLE_HDR

layout(std430, binding = 0) buffer HDRBuffer
{
	float screenBrightness;
};

out float avgBrightness;

void main(void) 
{
	// Send average screen brightness to fragment shader for HDR Mapping
	avgBrightness = screenBrightness;

	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
}