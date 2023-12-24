#version 430 core

out vec4 outputColor;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D noiseTexture;
uniform sampler2D matPropertiesMap;

uniform mat4 viewMat;
uniform mat4 transposeInverseViewMat;
uniform mat4 projMat;
uniform ivec2 screenSize;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
//int kernelSize = 64;
//float radius = 10.5;
//float bias = 0.025;

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

layout (std140) uniform SSAOSampleBuffer
{
	vec4 m_samples[64];
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

void main(void)
{
	// Calculate noise scale (based on screen size and noise texture size), so the noise texture will tile over the whole screen
	vec2 noiseScale = vec2(screenSize.x / 4.0, screenSize.y / 4.0);

	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Get pixel position in view space
	vec3 fragPos = getPositionInCameraSpace(texCoord);
	
	// Get pixel normal in view space
    vec3 normal = getNormalInCameraSpace(texCoord);
	
	// Get random direction vector
    vec3 randomVec = normalize(texture(noiseTexture, texCoord * noiseScale).xyz);
	
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
	
    float occlusion = 0.0;
	
    // Iterate over the sample kernel and calculate occlusion factor
    for(int i = 0; i < m_aoDataSet.m_numOfSamples; ++i)
    {
        // Get sample position
        vec3 samplePos = TBN * m_samples[i].xyz; // from tangent to view-space
        samplePos = fragPos + samplePos * m_aoDataSet.m_radius; 
        
        // Project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projMat * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // Get sample depth (distance from camera to the sample pixel)
		float sampleDepth = getPositionInCameraSpace(offset.xy).z;
        
        // Range check
        float rangeCheck = smoothstep(0.0, 1.0, m_aoDataSet.m_radius / abs(fragPos.z - sampleDepth));
		
		// Accumulate the occlusion result
        occlusion += (sampleDepth >= samplePos.z + m_aoDataSet.m_bias ? 1.0 : 0.0) * rangeCheck;           
    }
	
	// Average the occlusion result
    occlusion = 1.0 - (occlusion / m_aoDataSet.m_numOfSamples);
	
	// Apply ambient intensity
	occlusion = pow(occlusion, m_aoDataSet.m_PowExponent);
	    
	// Put the new occlusion value into the material properties texture
	vec4 matProperties = texture(matPropertiesMap, texCoord);
	matProperties.z *= occlusion;
	
	outputColor = matProperties;
}