#version 430 core

#define SHADOW_MAPPING 0

#define AVG_INTENDED_BRIGHTNESS 0.5
#define MIN_INTENDED_BRIGHTNESS 0.001
#define MAX_INTENDED_BRIGHTNESS 100.0

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10
#define PI 3.1415926535

#define NUM_OF_CASCADES 1
#define NUM_OF_PCF_SAMPLES 16

const float g_pcfSampleWeight = 1.0 / NUM_OF_PCF_SAMPLES;

const vec3 g_sunNoonColor = vec3(1.0, 1.0, 1.0);
const vec3 g_sunSetColor = vec3(1.0, 0.6, 0.2);

const float g_sunNoonIntensityMod = 1.0;
const float g_sunSetIntensityMod = 0.0;

// Poisson Disk PCF sampling
const vec2 poissonDisk[16] = vec2[]
( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

layout(location = 0) out vec4 colorBuffer;

in float avgBrightness;

struct DirectionalLight
{
    vec3 m_color;
    vec3 m_direction;
    float m_intensity;
};

struct PointLight
{
    vec3 m_color;
    vec3 m_position;
	
    float m_attenConstant;
    float m_attenLinear;
    float m_attenQuad;
    float m_intensity;
};
struct SpotLight
{
    vec3 m_color;
    vec3 m_position;
    vec3 m_direction;
	
    float m_attenConstant;
    float m_attenLinear;
    float m_attenQuad;
	
    float m_intensity;
    float m_cutoffAngle;
};

#if SHADOW_MAPPING
struct CascadedShadowMapDataSet
{
	mat4 m_lightSpaceMatrix;
	
	float m_cascadeplanedistance;
	float m_maxBias;
	float m_poissonSampleScale;
	float m_penumbraScale;
};
#endif

uniform sampler2D positionMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;
uniform sampler2D matPropertiesMap;
#if SHADOW_MAPPING
uniform sampler2DArray csmDepthMap;
uniform vec2 csmPenumbraScaleRange;
#endif

uniform mat4 lightMatrixTest;
uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform vec3 cameraPosVec;
uniform vec2 projPlaneRange; // x - zFar, y - zNear
uniform ivec2 screenSize;
uniform float gamma;
uniform vec3 ambientLightIntensity;	// x - directional, y - point, z - spot

uniform int numPointLights;
uniform int numSpotLights;

uniform DirectionalLight directionalLight;

// Using uniform buffer objects to pass light arrays and std140 layout for consistent variable spacing inside the buffer.
// Array size is fixed, but is updated partially, with only the lights that are being used, passing number of lights as uniform.
layout (std140) uniform PointLights
{
	PointLight pointLights[MAX_NUM_POINT_LIGHTS];
};
layout (std140) uniform SpotLights
{
	SpotLight spotLights[MAX_NUM_SPOT_LIGHTS];
};

#if SHADOW_MAPPING
layout (std140) uniform CSMDataSetBuffer
{
	CascadedShadowMapDataSet m_csmDataSet[NUM_OF_CASCADES];
};
#endif

layout (std430, binding = 0) buffer HDRBuffer
{
	float screenBrightness;
};

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

// Returns a random number based on a vec3 and an int.
float calcRandom(vec3 p_seed, int p_variable)
{
	vec4 seed = vec4(p_seed, p_variable);
	float dotProduct = dot(seed, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dotProduct) * 43758.5453);
}

// Linear interpolation in [0 1] range
float scaleLinear(float p_value, vec2 p_valueDomain) 
{
	return (p_value - p_valueDomain.x) / (p_valueDomain.y - p_valueDomain.x);
}

// Linear interpolation in the given range
float scaleLinear(float p_value, vec2 p_valueDomain, vec2 p_valueRange) 
{
	return mix(p_valueRange.x, p_valueRange.y, scaleLinear(p_value, p_valueDomain));
}

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
// by making overall brightness match the average intended brightness (AVG_INTENDED_BRIGHTNESS)
vec3 brightnessMapping(vec3 p_color, float p_exposure)
{
	return p_color * clamp(AVG_INTENDED_BRIGHTNESS / p_exposure, MIN_INTENDED_BRIGHTNESS, MAX_INTENDED_BRIGHTNESS);
}

float saturate(float p_value)
{
	return clamp(p_value, 0.0f, 1.0f);
}

// Calculates microfacet distribution (based on roughness)
// Using GGX normal distribution function
float DistributionGGX(vec3 p_normal, vec3 p_halfVector, float p_roughness)
{
	float roughnessSquaredSquared = p_roughness * p_roughness * p_roughness * p_roughness;
    float NdotH = max(dot(p_normal, p_halfVector), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float denominator = (NdotH2 * (roughnessSquaredSquared - 1.0) + 1.0);
    denominator = PI * denominator * denominator;
	
    return roughnessSquaredSquared / denominator;
}


// Geometry attenuation (Smith with Schlick's approximation)
float GeometrySchlickGGX(float p_NdotV, float p_roughness)
{
    float roughness = (p_roughness + 1.0);
    float k = (roughness * roughness) / 8.0;
	
    //float nominator   = p_NdotV;
    //float denominator = p_NdotV * (1.0 - k) + k;
	//return nominator / denominator;
	
    return p_NdotV / (p_NdotV * (1.0 - k) + k);
}

// Calculates geometric attenuation (or visibility term - self shadowing of microfacets)
float GeometrySmith(vec3 p_normal, vec3 p_fragToEye, vec3 L, float p_roughness)
{
    float NdotV = max(dot(p_normal, p_fragToEye), 0.0);
    float NdotL = max(dot(p_normal, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, p_roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, p_roughness);
	
    return ggx1 * ggx2;
}

// Calculates fresnel effect using Schlick's approximation
vec3 fresnelSchlick(float p_cosTheta, vec3 p_F0)
{
	return p_F0 + (1.0 - p_F0) * pow(clamp(1.0 - p_cosTheta, 0.0, 1.0), 5.0);
    //return p_F0 + (1.0 - p_F0) * pow(1.0 - p_cosTheta, 5.0);
} 

vec3 calcLightColor(const vec3 p_albedoColor, 
					const vec3 p_normal, 
					const vec3 p_fragToEye, 
					const vec3 p_lightColor, 
					const vec3 p_lightDirection, 
					const float p_lightDistance, 
					const vec3 p_F0, 
					const float p_roughness, 
					const float p_metalic, 
					const float p_ambientOcclusion, 
					const float p_ambientLightIntensity, 
					const float p_shadowFactor)
{	
	/*/ Get specular and diffuse lighting
	vec3 specularColor = LightingFuncGGX_REF(p_normal, p_fragToEye, p_lightDirection, p_roughnessSqrt, p_F0);
    vec3 diffuseColor = vec3(clamp(dot(p_normal, p_lightDirection), 0.0, 1.0));
	
	// Add specular and diffuse together, and multiply it by the color of the light
	return (specularColor + diffuseColor * p_diffuseAmount) * p_lightColor;*/
	
	// Calculate per-light radiance
	vec3 halfVector = normalize(p_fragToEye + p_lightDirection);
	float attenuation = 1.0 / (p_lightDistance * p_lightDistance);
	vec3 radiance = p_lightColor * attenuation;
	
	// Calculate Cook-Torrance BRDF
	float NDF = DistributionGGX(p_normal, halfVector, p_roughness);
	float G = GeometrySmith(p_normal, p_fragToEye, p_lightDirection, p_roughness);
	vec3 F = fresnelSchlick(clamp(dot(halfVector, p_fragToEye), 0.0, 1.0), p_F0);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - p_metalic;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(p_normal, p_fragToEye), 0.0) * max(dot(p_normal, p_lightDirection), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);
	
	// Combine diffuse, specular, radiance with albedo color and return it
	float NdotL = max(dot(p_normal, p_lightDirection), 0.0);
	
	// Add light color
#if SHADOW_MAPPING
	vec3 lightColor = (kD * p_albedoColor / PI + specular) * radiance * NdotL * p_shadowFactor;
#else
	vec3 lightColor = (kD * p_albedoColor / PI + specular) * radiance * NdotL;
#endif
	
	// Add ambient light
	lightColor += radiance * p_ambientOcclusion * p_ambientLightIntensity * (p_albedoColor) * (1.0 - p_metalic);
	
	return lightColor;
}

#if SHADOW_MAPPING
float calcCascadedShadow(vec3 p_worldPos, vec3 p_normal, vec3 p_lightDirection)
{
    vec4 fragPosViewSpace = viewMat * vec4(p_worldPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    // Calculate cascade layer
    int layer = NUM_OF_CASCADES;
    for (int i = 0; i < NUM_OF_CASCADES; ++i)
    {
        if (depthValue < m_csmDataSet[i].m_cascadeplanedistance)
        {
            layer = i;
            break;
        }
    }

	// Get fragment position in light space
    vec4 fragPosLightSpace = m_csmDataSet[layer].m_lightSpaceMatrix * vec4(p_worldPos, 1.0);
	
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
	
    // Calculate bias based on slope
	float bias = 0.05 * tan(acos(dot(p_normal, p_lightDirection)));
	bias = clamp(bias, 0.0, m_csmDataSet[layer].m_maxBias);
	
	// Calculate the poisson sampling scale
	float samplingScale = scaleLinear(depthValue, vec2(projPlaneRange.y, projPlaneRange.x), csmPenumbraScaleRange) * m_csmDataSet[layer].m_penumbraScale;
	
	// Start the fragment as fully lit
    float shadow = 1.0;
	
    // Perform PCF
	for(int i = 0 ; i < NUM_OF_PCF_SAMPLES; i++)
	{		
		// A random sample, based on the pixel's position in world space.
		// The position is rounded to the millimeter to avoid too much aliasing
		int index = int(16.0 * calcRandom(floor(p_worldPos.xyz * 1000.0), i)) % 16;
		
		// Get the fragments depth from the depth map
		float pcfDepth = texture(csmDepthMap, vec3(projCoords.xy + poissonDisk[index] / m_csmDataSet[layer].m_poissonSampleScale * samplingScale, layer)).r;
		
		// Check if the fragment is in the shadow (while applying bias)
		shadow -= (currentDepth - bias) > pcfDepth ? g_pcfSampleWeight : 0.0;
	}
	
    return shadow;
}
#endif

void main(void) 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	// Get the emissive texture color and convert it to linear space
	vec3 emissiveColor = pow(texture(emissiveMap, texCoord).xyz, vec3(gamma));
	// Get diffuse color (full-bright) from diffuse buffer and convert it to linear space
	vec3 diffuseColor = pow(texture(diffuseMap, texCoord).xyz, vec3(gamma));
	// Get pixel's position in world space
	vec3 worldPos = texture(positionMap, texCoord).xyz;
	// Get normal (in world space) and normalize it to minimize floating point approximation errors
	vec3 normal = normalize(texture(normalMap, texCoord).xyz);
	// Get material properties
	vec4 matProperties = texture(matPropertiesMap, texCoord).xyzw;
	// Calculate view direction (fragment to eye vector)
	vec3 fragmentToEye = normalize(cameraPosVec - worldPos);
	
	// Extract roughness, metalness and ambient occlusion values
	float roughnessSqrt = matProperties.x;
	float metalic = matProperties.y;
	float ambientOcclusion = matProperties.z;

	// Calculate F0, with minimum IOR as 0.04
	vec3 f0 = mix(vec3(0.04), diffuseColor, metalic);
	
	// Normalize direction light direction vector
	vec3 dirLightDirection =  normalize(directionalLight.m_direction);
	
	// Get dot product between the up vector (perpendicular to the ground) and directional light direction
	float dirLightFactor = dot(dirLightDirection, vec3(0.0, 1.0, 0.0)) + 0.03;
	
	// Initialize final color variable of this fragment
	vec3 finalLightColor = vec3(0.0);
	
	// Calculate directional light only if it is pointing from above the horizon
	if(dirLightFactor > 0.0)
	{
		float dirLightFactorSqrt = dirLightFactor;//sqrt(dirLightFactor);
		vec3 dirLightColor = mix(g_sunSetColor, g_sunNoonColor, dirLightFactorSqrt);
		float dirLightIntensity = directionalLight.m_intensity * mix(g_sunSetIntensityMod, g_sunNoonIntensityMod, dirLightFactorSqrt);
	
		// Perform cascaded shadow mapping
#if SHADOW_MAPPING
		float shadowFactor = calcCascadedShadow(worldPos, normal, dirLightDirection);
#else
		float shadowFactor = 1.0;
#endif
		
		// Add directional lighting
		finalLightColor += calcLightColor(
			diffuseColor, 
			normal, 
			fragmentToEye, 
			dirLightColor, 
			dirLightDirection, 
			1.0, 
			f0, 
			roughnessSqrt, 
			metalic,
			ambientOcclusion,
			ambientLightIntensity.x,
			shadowFactor) * dirLightIntensity;// * min(1.0, (dirLightFactor /* 100.0*/) + 0.02);
	}
		
	for(int i = 0; i < numPointLights; i++)
	{		
		// Get light direction, extract length from it and normalize for usage as direction vector
		vec3 lightDirection =  pointLights[i].m_position - worldPos;
		float lightDistance = length(lightDirection);
		lightDirection = normalize(lightDirection);
		
		// Light color multiplied by intensity and divided by attenuation
		finalLightColor += (calcLightColor(
			diffuseColor, 
			normal, 
			fragmentToEye, 
			pointLights[i].m_color, 
			lightDirection, 
			lightDistance, 
			f0, 
			roughnessSqrt, 
			metalic, 
			ambientOcclusion,
			ambientLightIntensity.y,
			1.0) * pointLights[i].m_intensity);
	}
	
	for(int i = 0; i < numSpotLights; i++)
	{			
		// Calculate direction from position of light to current pixel
		vec3 lightToFragment = normalize(worldPos - spotLights[i].m_position);
		
		// Get dot product of light direction and direction of light to pixel, and use it as a factor for light strength
		float spotLightFactor = dot(lightToFragment, spotLights[i].m_direction);
		
		// Early bail if pixel is outside of the cone of spot light
		if(spotLightFactor > spotLights[i].m_cutoffAngle)
		{
			// Get light direction, extract length from it and normalize for usage as direction vector
			vec3 lightDirection =  spotLights[i].m_position - worldPos;
			float lightDistance = length(lightDirection);
			lightDirection = normalize(lightDirection);
			
			// Light color multiplied by intensity
			vec3 lightColor = (calcLightColor(
				diffuseColor, 
				normal, 
				fragmentToEye, 
				spotLights[i].m_color, 
				lightDirection, 
				lightDistance, 
				f0, 
				roughnessSqrt, 
				metalic, 
				ambientOcclusion,
				ambientLightIntensity.z,
				1.0) * spotLights[i].m_intensity);
			
			// Light restriction from cone
			float coneAttenuation = (1.0 - (1.0 - spotLightFactor) * 1.0 / (1.0 - spotLights[i].m_cutoffAngle));
			
			finalLightColor += lightColor * coneAttenuation;
		}
	}
			
	colorBuffer = vec4(finalLightColor + emissiveColor, 1.0);
}