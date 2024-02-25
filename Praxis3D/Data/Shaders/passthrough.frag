/*
	Passthrough shader, fragment (passthrough.frag)
	Simply copies color from the input texture to the output texture.
*/
#version 430 core

#define TONEMAPPING_METHOD 0

out vec4 outputColor;

uniform ivec2 screenSize;

uniform sampler2D inputColorMap;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{	
	// Calculate screen-space texture coordinates, for buffer access
	const vec2 texCoord = calcTexCoord();

	// Get the color of the current fragment
	const vec3 fragmentColor = texture2D(inputColorMap, texCoord).xyz;
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}