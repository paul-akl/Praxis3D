#version 430 core

#define ENABLE_HDR

layout(std430, binding = 0) buffer HDRBuffer
{
	float screenBrightness;
};

out float avgBrightness;

uniform float eyeAdaptionRate;
uniform float deltaTimeS;
uniform ivec2 screenSize;

uniform sampler2D finalColorMap;

// Calculates the maximum texture mipmap level
float calcMaxMipmapLevel(vec2 p_textureSize)
{
	return 1 + floor(log2(max(p_textureSize.x, p_textureSize.y)));
}

// Returns the single brightest color
float getBrightestColor(vec3 p_color)
{
	return max(p_color.r, max(p_color.g, p_color.b));
}

// Averages RGB color into a single brightness value
float calcBrightnessLinear(vec3 p_color)
{
	return (p_color.x + p_color.y + p_color.z) / 3.0;
}

// Calculates a brightness value from a color
float calcBrightness(vec3 p_color)
{
	return dot(p_color, vec3(0.2126, 0.7152, 0.0722));
}

void main(void) 
{
	#ifdef ENABLE_HDR
	
	// Get maximum mipmap level (1x1) of a framebuffer
	float exposureMipmapLevel = calcMaxMipmapLevel(screenSize);
	// Get the current (previous frame) average brightness
	float avgBrightnessPrevFrame = calcBrightness(textureLod(finalColorMap, vec2(0.0), exposureMipmapLevel).xyz);
	// Perform a linear interpolation between current and previous brightness based on delta time
	screenBrightness = mix(screenBrightness, avgBrightnessPrevFrame, deltaTimeS / eyeAdaptionRate);
	
	#else
	
	// Set the average brightness to 0.5 so it does not affect the scene
	screenBrightness = 0.5;
	
	#endif
	
	// Send the average brightness value to the fragment shader
	avgBrightness = screenBrightness;
	
	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}