#version 430 core

//#define ENABLE_SIMPLE_TONE_MAPPING
//#define ENABLE_REINHARD_TONE_MAPPING
#define ENABLE_FILMIC_TONE_MAPPING

out vec4 outputColor;

uniform ivec2 screenSize;
uniform float gamma;

uniform sampler2D emissiveMap;
uniform sampler2D inputColorMap;

vec3 gammaCorrection(vec3 p_color, float p_gamma)
{
	return pow(p_color, vec3(1.0 / p_gamma));
}

vec3 simpleToneMapping(vec3 p_color)
{
    return exp(-1.0 / (2.72 * p_color + 0.15));
}

// Simple reinhard tone mapping
vec3 reinhardToneMapping(vec3 p_color)
{
	return p_color / (p_color + vec3(1.0));
}
	
// Filmic tone mapping using an algorithm created by John Hable for Uncharted 2
vec3 filmicToneMapping(vec3 p_color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;

	return ((p_color*(A*p_color+C*B)+D*E)/(p_color*(A*p_color+B)+D*F))-E/F;
}

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
	fragmentColor += texture(emissiveMap, texCoord).xyz;
		
	#ifdef ENABLE_TONE_MAPPING
	// Perform simple tonemapping on the final color
	fragmentColor = simpleToneMapping(fragmentColor);
	#endif
	
	#ifdef ENABLE_REINHARD_TONE_MAPPING
	// Perform reinhard tonemapping on the final color
	fragmentColor = reinhardToneMapping(fragmentColor);
	#endif
	
	#ifdef ENABLE_FILMIC_TONE_MAPPING
	// Perform filmic tonemapping on the final color
	fragmentColor = filmicToneMapping(fragmentColor);
	#endif
	
	// Perform gamma correction as the last step of the fragment color
	fragmentColor = gammaCorrection(fragmentColor, gamma);
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}