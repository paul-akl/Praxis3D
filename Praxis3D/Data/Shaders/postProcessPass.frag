#version 430 core

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
	vec2 texCoord = calcTexCoord();
	
	// Get the current fragment color
	vec3 fragmentColor = texture(inputColorMap, texCoord).xyz;
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}