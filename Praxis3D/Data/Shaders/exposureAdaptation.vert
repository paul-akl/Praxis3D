/*
	Exposure adaptation shader, vertex (exposureAdaptation.vert)
	Adjusts exposure by converting the color from RGB to Yxy color space and adjusting the luminosity component based on average scene luminance value.
*/
#version 430 core

void main(void) 
{	
	// Determine texture coordinates
	const vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.5, 1.0);
}