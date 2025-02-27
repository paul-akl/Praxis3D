/*
	Final pass shader, vertex (finalPass.vert)
	Performs FXAA anti-aliasing. Also used to copy color data to the default framebuffer to show on screen.
*/
#version 430 core

void main(void) 
{	
	// Determine texture coordinates
	const vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.5, 1.0);
}