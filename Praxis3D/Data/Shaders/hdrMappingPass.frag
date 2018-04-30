#version 430 core

//#define AVG_INTENDED_BRIGHTNESS 0.2
#define MIN_INTENDED_BRIGHTNESS 0.01
#define MAX_INTENDED_BRIGHTNESS 10.0

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10
#define PI 3.1415926535

/*
layout(std430, binding = 0) buffer HDRBuffer
{
	float screenBrightness;
};*/

layout(location = 0) out vec4 emissiveBuffer;
layout(location = 1) out vec4 colorBuffer;

in float avgBrightness;

uniform sampler2D emissiveMap;
uniform sampler2D inputColorMap;

uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform vec3 cameraPosVec;
uniform ivec2 screenSize;
uniform float eyeAdaptionIntBrightness;
uniform float gamma;

float getBrightestColor(vec3 p_color)
{
	return max(p_color.x, max(p_color.y, p_color.z));
}

// Calculates a brightness value from a color
float calcBrightness(vec3 p_color)
{
	return dot(p_color, vec3(0.2126, 0.7152, 0.0722));
}

// Adjusts an RGB color based on the average brightness (exposure)
// by making overall brightness match the average intended brightness (eyeAdaptionIntBrightness)
vec3 brightnessMapping(vec3 p_color, float p_exposure)
{
	return p_color * clamp(eyeAdaptionIntBrightness / p_exposure, MIN_INTENDED_BRIGHTNESS, MAX_INTENDED_BRIGHTNESS);
}

float saturate(float p_value)
{
	return clamp(p_value, 0.0f, 1.0f);
}

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void) 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	// Get the emissive texture color
	vec3 emissiveColor = /*pow(*/texture(emissiveMap, texCoord).xyz/*, vec3(gamma))*/;
	// Get the emissive texture color
	vec3 fragmentColor = texture(inputColorMap, texCoord).xyz;
	
	// Adjust the fragment brightness based on average
	fragmentColor = brightnessMapping(fragmentColor, avgBrightness);
	
	// If a fragment brightness exceeds a value of 1.0 after an HDR Mapping, 
	// add it to the emissive buffer, as a bloom effect
	if(calcBrightness(fragmentColor) > 1.0)
		emissiveColor += fragmentColor;
	
	emissiveBuffer = vec4(emissiveColor, 1.0);
	colorBuffer = vec4(fragmentColor, 1.0);
}