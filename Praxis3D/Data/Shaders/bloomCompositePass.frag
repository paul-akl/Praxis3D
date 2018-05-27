#version 430 core

out vec4 outputColor;

uniform ivec2 screenSize;

uniform sampler2D emissiveMap;
uniform sampler2D inputColorMap;
uniform sampler2D lensDirtTexture;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Perform gamma correction on the color from the final framebuffer
	vec3 fragmentColor = texture(inputColorMap, texCoord).xyz;
	
	// Add emissive color (which is generated in a blur pass)
	// And apply lens dirt texture
	fragmentColor += texture(emissiveMap, texCoord).xyz * min(texture(lensDirtTexture, texCoord).r, 1.0);
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}