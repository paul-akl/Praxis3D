#version 430 core

out vec4 outputColor;

uniform sampler2D inputColorMap;
uniform sampler2D matPropertiesMap;

uniform ivec2 screenSize;
uniform vec2 hbaoBlurVerticalInvResDirection;
uniform int hbaoBlurNumOfSamples;
uniform float hbaoBlurSharpness;
  
vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

float blurFunction(vec2 p_texCoord, float p_offset, float p_depth, inout float p_totalWeight)
{
	vec2  occlusionAndDepth = texture2D(inputColorMap, p_texCoord).xy;
	float occlusion = occlusionAndDepth.x;
	float depth = occlusionAndDepth.y;

	const float BlurSigma = float(hbaoBlurNumOfSamples) * 0.5;
	const float BlurFalloff = 1.0 / (2.0 * BlurSigma * BlurSigma);

	float depthDifference = (depth - p_depth) * hbaoBlurSharpness;
	float weight = exp2(-p_offset * p_offset * BlurFalloff - depthDifference * depthDifference);
	p_totalWeight += weight;

	return occlusion * weight;
}

void main()
{	
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	vec2 occlusionAndDepth = texture(inputColorMap, texCoord).xy;
	float occlusionAtCenter = occlusionAndDepth.x;
	float depth = occlusionAndDepth.y;

	float totalOcclusion = occlusionAtCenter;
	float totalWeight = 1.0;

	for(float r = 1; r <= hbaoBlurNumOfSamples; ++r)
	{
		vec2 offsetTexCoord = texCoord + hbaoBlurVerticalInvResDirection * r;
		totalOcclusion += blurFunction(offsetTexCoord, r, depth, totalWeight);  
	}

	for(float r = 1; r <= hbaoBlurNumOfSamples; ++r)
	{
		vec2 offsetTexCoord = texCoord - hbaoBlurVerticalInvResDirection * r;
		totalOcclusion += blurFunction(offsetTexCoord, r, depth, totalWeight);  
	}

	// Multiply the existing AO value (from textures) by the calculated occlusion value
	vec4 matProperties = texture(matPropertiesMap, texCoord);
	matProperties.z = (matProperties.z + totalOcclusion / totalWeight) * 0.5;
	//matProperties.z *= c_total / w_total;
	
    outputColor = matProperties;
}