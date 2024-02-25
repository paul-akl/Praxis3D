/*
	Light pass shader, fragment (lightPass.frag)
	Calculates lighting by using Cook-Torrance BRDF:
		Microfacet distribution (based on roughness) using GGX distribution function
		Geometry attenuation using Smith with Schlick's approximation
		Fresnel effect using Schlick's approximation
	Performs Cascaded Shadow Mapping, using PCF filtering with Poisson Disk sampling and automatic bias based on slope
*/
#version 430 core

void main(void) 
{
	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}