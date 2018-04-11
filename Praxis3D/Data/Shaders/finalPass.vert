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

// Averages RGB color into a single brightness value
float averageColors(vec3 p_color)
{
	return (p_color.x + p_color.y + p_color.z) / 3.0;
}

void main(void) 
{
	#ifdef ENABLE_HDR
	
	// Get maximum mipmap level (1x1) of a framebuffer
	float exposureMipmapLevel = calcMaxMipmapLevel(screenSize);
	// Get the current (previous frame) average brightness
	float avgBrightnessPrevFrame = averageColors(textureLod(finalColorMap, vec2(0.0), exposureMipmapLevel).xyz);
	// Perform a linear interpolation between current and previous brightness based on delta time
	screenBrightness = mix(screenBrightness, avgBrightnessPrevFrame, deltaTimeS * eyeAdaptionRate);
	// Send average brightness to the fragment shader
	avgBrightness = screenBrightness;
	
	#else
	
	// Set the average brightness to 0.5 so it does not affect the scene
	avgBrightness = 0.5;
	
	#endif
	
	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}