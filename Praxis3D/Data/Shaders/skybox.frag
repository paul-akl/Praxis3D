#version 430 core

layout(location = 1) out vec4 finalBuffer;

in vec3 cubemapCoord;

//uniform sampler2D diffuseTexture;
uniform samplerCube staticEnvMap;
uniform float gamma;

void main(void) 
{	
	// Convert the color from skybox texture to linear space (because it gets gamma-corrected at the final pass)
	vec3 color = pow(texture(staticEnvMap, cubemapCoord).rgb, vec3(gamma));
	
	// Write the color from skybox texture to the final buffer
	finalBuffer = vec4(0.0, 1.0, 0.0, 1.0);//vec4(color, 1.0);
}