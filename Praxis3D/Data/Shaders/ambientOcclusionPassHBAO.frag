#version 430 core

#define PI 3.1415926535
#define AO_RANDOMTEX_SIZE 4

out vec4 outputColor;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D noiseTexture;

uniform mat4 viewMat;
uniform mat4 transposeInverseViewMat;
uniform ivec2 screenSize;

struct AODataSet
{
	float m_RadiusToScreen;	// radius
	float m_radius;
	float m_NegInvR2;     	// radius * radius
	float m_NDotVBias;

	vec2 m_InvFullResolution;
	float m_AOMultiplier;
	float m_PowExponent;

	float m_bias;
	int m_numOfDirections;
	int m_numOfSamples;
	int m_numOfSteps;
};

layout (std140) uniform AODataSetBuffer
{
	AODataSet m_aoDataSet;
};
  
vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

// Converts the position from position g-buffer that is in world-space to position in view-space
vec3 getPositionInCameraSpace(vec2 p_texCoord)
{
	vec4 fragPos = vec4(texture(positionMap, p_texCoord).xyz, 1.0);
	fragPos = viewMat * fragPos;
	return fragPos.xyz;
}

// Converts the normal from normal g-buffer that is in world-space to normal in view-space
vec3 getNormalInCameraSpace(vec2 p_texCoord)
{
	vec4 normal = vec4(texture(normalMap, p_texCoord).xyz, 1.0);
	normal = transposeInverseViewMat * normal;
	return normalize(normal.xyz);
}

// Get the random (jitter) value for the current pixel
vec4 getRandom(void)
{
	// Get the current jitter vector from the per-pass constant buffer
	return texture(noiseTexture, (gl_FragCoord.xy / AO_RANDOMTEX_SIZE));
}

// Rotate the given direction by the cosine / sine
vec2 rotateDirection(vec2 p_direction, vec2 p_cosSin)
{
	return vec2(p_direction.x * p_cosSin.x - p_direction.y * p_cosSin.y, 
				p_direction.x * p_cosSin.y + p_direction.y * p_cosSin.x);
}

// Calculate falloff by the given distance
float falloff(float p_distanceSquare)
{
	// 1 scalar mad instruction
	return p_distanceSquare * m_aoDataSet.m_NegInvR2 + 1.0;
}

//----------------------------------------------------------------------------------
// p_viewPosition = view-space position at the kernel center
// p_viewNormal = view-space normal at the kernel center
// p_sampleViewPosition = view-space position of the current sample
//----------------------------------------------------------------------------------
float computeAmbientOcclusion(vec3 p_viewPosition, vec3 p_viewNormal, vec3 p_sampleViewPosition)
{
	vec3 viewRay = p_sampleViewPosition - p_viewPosition;
	float VdotV = dot(viewRay, viewRay);
	float NdotV = dot(p_viewNormal, viewRay) * 1.0/sqrt(VdotV);

	// Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
	return clamp(NdotV - m_aoDataSet.m_NDotVBias, 0.0, 1.0) * clamp(falloff(VdotV), 0.0, 1.0);
}

float computeCoarseAmbientOcclusion(vec2 p_texCoord, float p_radiusPixels, vec4 p_randomVec, vec3 p_viewPosition, vec3 p_viewNormal)
{
	// Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
	float stepSizePixels = p_radiusPixels / (m_aoDataSet.m_numOfSteps + 1);

	const float alpha = 2.0 * PI / m_aoDataSet.m_numOfDirections;
	float occlusion = 0;

	// Go over each direction
	for(float DirectionIndex = 0; DirectionIndex < m_aoDataSet.m_numOfDirections; ++DirectionIndex)
	{
		float angle = alpha * DirectionIndex;

		// Compute normalized 2D direction
		vec2 direction = rotateDirection(vec2(cos(angle), sin(angle)), p_randomVec.xy);

		// Jitter starting sample within the first step
		float rayPixels = (p_randomVec.z * stepSizePixels + 1.0);
		
		// Go over each step
		for(float stepIndex = 0; stepIndex < m_aoDataSet.m_numOfSteps; ++stepIndex)
		{
			vec2 snappedTexCoord = round(rayPixels * direction) * m_aoDataSet.m_InvFullResolution + p_texCoord;
			vec3 sampleViewPosition = getPositionInCameraSpace(snappedTexCoord);
			
			// March the ray further
			rayPixels += stepSizePixels;
			
			// Accumulate the occlusion result
			occlusion += computeAmbientOcclusion(p_viewPosition, p_viewNormal, sampleViewPosition);
		}
	}

	// Average the occlusion result
	occlusion *= m_aoDataSet.m_AOMultiplier / (m_aoDataSet.m_numOfDirections * m_aoDataSet.m_numOfSteps);
	
	// Make sure the occlusion is not clipping
	return clamp(1.0 - occlusion * 2.0, 0.0, 1.0);
}

void main()
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Get pixel position in view space
	vec3 viewPosition = getPositionInCameraSpace(texCoord);
	
	// Get pixel normal in view space
	vec3 viewNormal = getNormalInCameraSpace(texCoord);

	// Calculate the radius for the occlusion calculation
	float radiusPixels = m_aoDataSet.m_RadiusToScreen / viewPosition.z;

	// Get jitter (random) vector for the current pixel
	vec4 randomVec = getRandom();

	// Perform ambient occlusion
	float occlusionValue = computeCoarseAmbientOcclusion(texCoord, radiusPixels, randomVec, viewPosition, viewNormal);
	
	outputColor = vec4(pow(occlusionValue, m_aoDataSet.m_PowExponent), viewPosition.z, 0.0, 0.0);
}