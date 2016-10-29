#version 450 core

out vec4 outputColor;

uniform sampler2D finalColorMap;
uniform ivec2 screenSize;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
		
	// Write the color from the final framebuffer to the default framebuffer
	outputColor = vec4(texture(finalColorMap, texCoord).xyz, 1.0);
}