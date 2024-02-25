/*
	Exposure adaptation shader, fragment (exposureAdaptation.frag)
	Adjusts exposure by converting the color from RGB to Yxy color space and adjusting the luminosity component based on average scene luminance value.
*/
#version 430 core

out vec4 outputColor;

uniform ivec2 screenSize;
uniform float luminanceMultiplier;

uniform sampler2D inputColorMap;
uniform sampler2D averageLuminanceTexture;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

float log10(const float p_x)
{
    //log10(x) = log(x) / log(10) = (1 / log(10)) * log(x)
    const float d = 1.0 / log(10.0);
    return d * log(p_x);
}

vec3 splatVec3(const float p_x) 
{ 
	return vec3(p_x, p_x, p_x); 
}

// Convert RGB to CIE 1931 XYZ color space
vec3 convertRGB2XYZ(const vec3 p_rgb)
{
	// Reference:
	// RGB/XYZ Matrices
	// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
	vec3 xyz;
	xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), p_rgb);
	xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), p_rgb);
	xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), p_rgb);
	return xyz;
}

// Convert XYZ to RGB color space
vec3 convertXYZ2RGB(const vec3 p_xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), p_xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), p_xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), p_xyz);
	return rgb;
}

// Convert XYZ to Yxy color space
vec3 convertXYZ2Yxy(const vec3 p_xyz)
{
	// Reference:
	// http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
	float inv = 1.0 / dot(p_xyz, vec3(1.0, 1.0, 1.0));
	return vec3(p_xyz.y, p_xyz.x * inv, p_xyz.y * inv);
}

// Convert Yxy to XYZ color space
vec3 convertYxy2XYZ(const vec3 p_Yxy)
{
	// Reference:
	// http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
	vec3 xyz;
	xyz.x = p_Yxy.x * p_Yxy.y / p_Yxy.z;
	xyz.y = p_Yxy.x;
	xyz.z = p_Yxy.x * (1.0 - p_Yxy.y - p_Yxy.z) / p_Yxy.z;
	return xyz;
}

// Convert RGB to Yxy color space
vec3 convertRGB2Yxy(const vec3 p_rgb)
{
	return convertXYZ2Yxy(convertRGB2XYZ(p_rgb));
}

// Convert Yxy to RGB color space
vec3 convertYxy2RGB(const vec3 p_Yxy)
{
	return convertXYZ2RGB(convertYxy2XYZ(p_Yxy));
}

void main(void)
{	
	// Calculate screen-space texture coordinates, for buffer access
	const vec2 texCoord = calcTexCoord();

	// Get the color of the current fragment
	vec3 fragmentColor = texture2D(inputColorMap, texCoord).xyz;

	// Get the average scene luminance
	const float luminance = texture2D(averageLuminanceTexture, vec2(0.0, 0.0)).r * luminanceMultiplier;
	
	// Do not adjust the exposure on black areas, as it can introduce
	if(fragmentColor != vec3(0.0))
	{
		// Perform exposure compensation by converting the color from RGB to Yxy color space and adjusting the luminosity component
		vec3 colorYxy = convertRGB2Yxy(fragmentColor);
		colorYxy.x /= (9.6 * luminance + 0.0001);
		fragmentColor = convertYxy2RGB(colorYxy);
		//fragmentColor = max(convertYxy2RGB(colorYxy), vec3(0.0));
	}
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}