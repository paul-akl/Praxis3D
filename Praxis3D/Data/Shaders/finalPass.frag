#version 430 core

//#define ENABLE_SIMPLE_TONE_MAPPING
//#define ENABLE_REINHARD_TONE_MAPPING
#define ENABLE_FILMIC_TONE_MAPPING

#define AVG_INTENDED_BRIGHTNESS 0.5
#define MIN_INTENDED_BRIGHTNESS 0.005
#define MAX_INTENDED_BRIGHTNESS 5.0


in float avgBrightness;

out vec4 outputColor;

uniform sampler2D finalColorMap;
uniform ivec2 screenSize;
uniform float gamma;

// Averages RGB color into a single brightness value
float averageColors(vec3 p_color)
{
	return (p_color.x + p_color.y + p_color.z) / 3.0;
}

vec3 gammaCorrection(vec3 p_color, float p_gamma)
{
	return pow(p_color, vec3(1.0 / p_gamma));
}

// Adjusts an RGB color based on the average brightness (exposure)
// by making overall brightness match the average intended brightness (AVG_INTENDED_BRIGHTNESS)
vec3 brightnessMapping(vec3 p_color, float p_exposure)
{
	return p_color * clamp(AVG_INTENDED_BRIGHTNESS / p_exposure, MIN_INTENDED_BRIGHTNESS, MAX_INTENDED_BRIGHTNESS);
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
	vec3 color = texture(finalColorMap, texCoord).xyz;
		
	// Adjust the fragment brightness based on average
	color = brightnessMapping(color, avgBrightness);
	
	#ifdef ENABLE_TONE_MAPPING
	// Perform simple tonemapping on the final color
	color = simpleToneMapping(color);
	#endif
	
	#ifdef ENABLE_REINHARD_TONE_MAPPING
	// Perform reinhard tonemapping on the final color
	color = reinhardToneMapping(color);
	#endif
	
	#ifdef ENABLE_FILMIC_TONE_MAPPING
	// Perform filmic tonemapping on the final color
	color = filmicToneMapping(color);
	#endif
	
	// Perform gamma correction as the last step of the fragment color
	color = gammaCorrection(color, gamma);
	
	// Write the color to the default framebuffer
	outputColor = vec4(color, 1.0);
}

	//vec3 filmicToneMapping2(vec3 p_fragmentColor)
	//{
	//float3 texColor = tex2D(Texture0, texCoord ); // Tex Read
	// Filmic Curve
	//vec3 color = max(vec3(0.0), p_fragmentColor - 0.004); 
	//vec3 retColor = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	//return retColor;
	//}
	//vec3 color = pow(texture(finalColorMap, texCoord).xyz, vec3(1.0 / gamma));
	//vec3 lumVec = textureLod(finalColorMap, texCoord, exposureMipmapLevel).xyz;
	//float lum = (lumVec.x + lumVec.y + lumVec.z) / 3.0;
	//lum = max(lumVec.x, max(lumVec.y, lumVec.z));
	//float lum = lumVec.x;
	//color = pow(uncharted2Tonemap(color), vec3(1.0 / gamma));
	//color = uncharted2Tonemap(color);
    // Exposure tone mapping
    //color = vec3(1.0) - exp(-color * min(1.0 - lum, 0.5));
    //color = vec3(1.0) - exp(-color * (0.5 / max(lum, 0.2)));
	//color = vec3(0.5) / lum;
	//color = color * (1.0 / max(lum, 1.0));
	//color = color * clamp(0.5 / lum, 0.05, 5.0);
	//color = color / (color + vec3(1.0));
    //color = vec3(1.0) - exp(-color * (0.5 / lum));
	//color = color / max(lum, 1.0);
	//color = color / lum;
	//color = filmicToneMapping(color);
	//color = color * clamp(0.5 / lum, 0.05, 5.0);