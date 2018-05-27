#version 430 core

//#define AVG_INTENDED_BRIGHTNESS 0.2
#define MIN_INTENDED_BRIGHTNESS 0.05
#define MAX_INTENDED_BRIGHTNESS 5.0

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

// Calculates a brightness value (luma) from a color
float calcLuma(vec3 p_color)
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
	emissiveColor = brightnessMapping(emissiveColor, avgBrightness);
	
	// Calculate luminance of the fragment after HDR mapping
	float luminance = max(0.0, calcLuma(fragmentColor) - 1.0);
	
	// Get bright fragments
	vec3 fragmentBrightness = min(luminance * fragmentColor, fragmentColor);
	
	// Add bright fragments to the emissive buffer for bloom effect
	emissiveColor += fragmentBrightness;
	//fragmentColor = max(fragmentColor * (1.0 - luminance), vec3(0.0));
	//fragmentColor = max(fragmentColor - fragmentBrightness, vec3(0.0));
	
	emissiveBuffer = vec4(emissiveColor, 1.0);
	colorBuffer = vec4(fragmentColor, 1.0);
}