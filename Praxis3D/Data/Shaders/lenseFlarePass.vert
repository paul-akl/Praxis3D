#version 430 core

flat out float aspectRatio;

uniform ivec2 screenSize;

void main(void) 
{	
	// Calculate aspect ratio 
	aspectRatio = screenSize.x / screenSize.y;

	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}