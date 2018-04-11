#version 430 core

//#define ENABLE_TONE_MAPPING

out vec4 outputColor;

uniform sampler2D finalColorMap;
uniform ivec2 screenSize;
uniform float gamma;

#ifdef ENABLE_TONE_MAPPING
vec3 simpleToneMapping(vec3 p_color, float p_gamma)
{
    return pow(exp(-1.0 / (2.72 * p_color + 0.15)), vec3(1.0 / p_gamma));
}
#endif

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	float mipmapLevel = textureQueryLod(myTexture, textureCoord).x;
    fragColor = textureLod(myTexture, textureCoord, mipmapLevel);

	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Perform gamma correction on the color from the final framebuffer
	vec3 color = pow(texture(finalColorMap, texCoord).xyz, vec3(1.0 / gamma));
	
	#ifdef ENABLE_TONE_MAPPING
	// Perform simple tonemapping on the final color
	color = simpleToneMapping(color, gamma);
	#endif
	
	color = color / (color + vec3(1.0));
	
	// Write the color to the default framebuffer
	outputColor = vec4(color, 1.0);
}