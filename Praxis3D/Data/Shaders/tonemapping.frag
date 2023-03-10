#version 430 core

out vec4 outputColor;

uniform ivec2 screenSize;
uniform float gamma;
uniform int tonemapMethod;

uniform sampler2D inputColorMap;
uniform sampler2D averageLuminanceTexture;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

float log10(float p_x)
{
    //log10(x) = log(x) / log(10) = (1 / log(10)) * log(x)
    const float d = 1.0 / log(10.0);
    return d * log(p_x);
}

vec3 splatVec3(float p_x) 
{ 
	return vec3(p_x, p_x, p_x); 
}

// Convert RGB to CIE 1931 XYZ color space
vec3 convertRGB2XYZ(vec3 p_rgb)
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
vec3 convertXYZ2RGB(vec3 p_xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), p_xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), p_xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), p_xyz);
	return rgb;
}

// Convert XYZ to Yxy color space
vec3 convertXYZ2Yxy(vec3 p_xyz)
{
	// Reference:
	// http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
	float inv = 1.0 / dot(p_xyz, vec3(1.0, 1.0, 1.0));
	return vec3(p_xyz.y, p_xyz.x * inv, p_xyz.y * inv);
}

// Convert Yxy to XYZ color space
vec3 convertYxy2XYZ(vec3 p_Yxy)
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
vec3 convertRGB2Yxy(vec3 p_rgb)
{
	return convertXYZ2Yxy(convertRGB2XYZ(p_rgb));
}

// Convert Yxy to RGB color space
vec3 convertYxy2RGB(vec3 p_Yxy)
{
	return convertXYZ2RGB(convertYxy2XYZ(p_Yxy));
}

vec3 gammaCorrectionAccurate(vec3 p_color)
{
	vec3 lo  = p_color * 12.92;
	vec3 hi  = pow(abs(p_color), splatVec3(1.0 / 2.4) ) * 1.055 - 0.055;
	vec3 rgb = mix(hi, lo, vec3(lessThanEqual(p_color, splatVec3(0.0031308))));
	return rgb;
}

vec3 gammaCorrection(vec3 p_color, float p_gamma)
{
	return pow(p_color, vec3(1.0 / p_gamma));
}

vec3 compensateByLuminance(vec3 p_color, float p_luminance)
{
	float KeyValue = 1.03 - (2.0 / (log10(p_luminance + 1.0) + 2.0));
    float ExposureValue = log2(KeyValue / p_luminance);// + _ManualBias;
    return p_color * exp2(ExposureValue);
}

// Simple reinhard tone mapping
vec3 tonemap_reinhard(vec3 p_color)
{
	return p_color / (p_color + vec3(1.0));
}

// Simple reinhard tone mapping with white point
vec3 tonemap_reinhardWhitePoint(vec3 p_color, float p_whiteSqr)
{
	return (p_color * (1.0 + p_color / p_whiteSqr)) / (1.0 + p_color);
}
	
// Filmic tone mapping using an algorithm created by John Hable for Uncharted 2
vec3 tonemap_filmic(vec3 p_color)
{
	// http://www.gdcvault.com/play/1012459/Uncharted_2__HDR_Lighting
	// http://filmicgames.com/archives/75 - the coefficients are from here
	float A = 0.15; // Shoulder Strength
	float B = 0.50; // Linear Strength
	float C = 0.10; // Linear Angle
	float D = 0.20; // Toe Strength
	float E = 0.02; // Toe Numerator
	float F = 0.30; // Toe Denominator
	
	return ((p_color * (A * p_color + C * B) + D * E) / (p_color * (A * p_color + B) + D * F)) - E / F; // E/F = Toe Angle
}

// Filimic tonemapping from Uncharted 2
vec3 tonemap_Uncharted2(vec3 p_color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	p_color = ((p_color * (A * p_color + C * B) + D * E) / (p_color * (A * p_color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	p_color /= white;
	return p_color;
}

// Unreal engine 3 tonemapping
float tonemap_Unreal(float p_color) 
{
	// Unreal 3, Documentation: "Color Grading"
	// Adapted to be close to Tonemap_ACES, with similar range
	// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
	return p_color / (p_color + 0.155) * 1.019;
}

// ACES tonemapping
float tonemap_ACES(float p_color) 
{
	// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	
	return (p_color * (a * p_color + b)) / (p_color * (c * p_color + d) + e);
}

// Lottes 2016 tonemapping
float tonemap_Lottes(float p_color) 
{
	// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
	const float a = 1.6;
	const float d = 0.977;
	const float hdrMax = 8.0;
	const float midIn = 0.18;
	const float midOut = 0.267;

	// Can be precomputed
	const float b = (-pow(midIn, a) + pow(hdrMax, a) * midOut) / ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
	const float c = (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) / ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

	return pow(p_color, a) / (pow(p_color, a * d) * b + c);
}

float calculateUchimura(float p_color, float p_maxBrightness, float p_contrast, float p_start, float p_length, float p_black, float p_pedestal)
{
	// Uchimura 2017, "HDR theory and practice"
	// Math: https://www.desmos.com/calculator/gslcdxvipg
	// Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
	float l0 = ((p_maxBrightness - p_start) * p_length) / p_contrast;
	float L0 = p_start - p_start / p_contrast;
	float L1 = p_start + (1.0 - p_start) / p_contrast;
	float S0 = p_start + l0;
	float S1 = p_start + p_contrast * l0;
	float C2 = (p_contrast * p_maxBrightness) / (p_maxBrightness - S1);
	float CP = -C2 / p_maxBrightness;

	float w0 = 1.0 - smoothstep(0.0, p_start, p_color);
	float w2 = step(p_start + l0, p_color);
	float w1 = 1.0 - w0 - w2;

	float T = p_start * pow(p_color / p_start, p_black) + p_pedestal;
	float S = p_maxBrightness - (p_maxBrightness - S1) * exp(CP * (p_color - S0));
	float L = p_start + p_contrast * (p_color - p_start);

	return T * w0 + L * w1 + S * w2;
}

// Uchimura 2017 tonemapping
float tonemap_Uchimura(float p_color)
{
	const float P = 1.0;  // max display brightness
	const float a = 1.0;  // contrast
	const float m = 0.22; // linear section start
	const float l = 0.4;  // linear section length
	const float c = 1.33; // black
	const float b = 0.0;  // pedestal
	return calculateUchimura(p_color, P, a, m, l, c, b);
}

void main(void)
{	
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();

	// Get the color of the current fragment
	vec3 fragmentColor = texture2D(inputColorMap, texCoord).xyz;

	// Get the average scene luminance
	float luminance = texture2D(averageLuminanceTexture, vec2(0.0, 0.0)).r;
	
	// Perform exposure compensation by converting the color from RGB to Yxy color space and adjusting the luminosity component
	vec3 colorYxy = convertRGB2Yxy(fragmentColor);
	colorYxy.x /= (9.6 * luminance + 0.0001);
	fragmentColor = convertYxy2RGB(colorYxy);
	
	// Perform tonemapping, choosing the method based on a uniform
	switch(tonemapMethod) 
	{
		// No tonemapping
		case 0:
		
		break;
		
		// Simple reinhard tonemapping
		case 1:
			fragmentColor = tonemap_reinhard(fragmentColor);
			fragmentColor.x = tonemap_ACES(fragmentColor.x);
			fragmentColor.y = tonemap_ACES(fragmentColor.y);
			fragmentColor.z = tonemap_ACES(fragmentColor.z);
		break;
			
		// Reinhard with white point tonemapping
		case 2:	
			float whitePoint = 3.0f;
			whitePoint = whitePoint * whitePoint;
			
			fragmentColor = tonemap_reinhardWhitePoint(fragmentColor, whitePoint);
		break;
		
		// Filmic tonemapping
		case 3:
			fragmentColor = tonemap_filmic(fragmentColor);
		break;
		
		// Uncharted 2 tonemapping
		case 4:
			fragmentColor = tonemap_Uncharted2(fragmentColor);
		break;
		
		// Unreal 3 tonemapping
		case 5:
			fragmentColor.x = tonemap_Unreal(fragmentColor.x);
			fragmentColor.y = tonemap_Unreal(fragmentColor.y);
			fragmentColor.z = tonemap_Unreal(fragmentColor.z);
		break;
		
		// ACES tonemapping
		case 6:
			fragmentColor.x = tonemap_ACES(fragmentColor.x);
			fragmentColor.y = tonemap_ACES(fragmentColor.y);
			fragmentColor.z = tonemap_ACES(fragmentColor.z);
		break;
		
		// Lottes tonemapping
		case 7:
			fragmentColor.x = tonemap_Lottes(fragmentColor.x);
			fragmentColor.y = tonemap_Lottes(fragmentColor.y);
			fragmentColor.z = tonemap_Lottes(fragmentColor.z);
		break;
		
		// Uchimura tonemapping
		case 8:
			fragmentColor.x = tonemap_Uchimura(fragmentColor.x);
			fragmentColor.y = tonemap_Uchimura(fragmentColor.y);
			fragmentColor.z = tonemap_Uchimura(fragmentColor.z);
		break;
	}
	
	// Perform gamma correction
	fragmentColor = gammaCorrectionAccurate(fragmentColor);

	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}