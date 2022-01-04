#version 430 core

#define AVG_INTENDED_BRIGHTNESS 0.5
#define MIN_INTENDED_BRIGHTNESS 0.001
#define MAX_INTENDED_BRIGHTNESS 100.0

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10
#define PI 3.1415926535

const vec3 g_sunNoonColor = vec3(1.0, 1.0, 1.0);
const vec3 g_sunSetColor = vec3(1.0, 0.6, 0.2);

const float g_sunNoonIntensityMod = 1.0;
const float g_sunSetIntensityMod = 0.0;

layout(std430, binding = 0) buffer HDRBuffer
{
	float screenBrightness;
};

//layout(location = 0) out vec4 emissiveBuffer;
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

uniform sampler2D positionMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;
uniform sampler2D matPropertiesMap;

//uniform samplerCube staticEnvMap;

uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform vec3 cameraPosVec;
uniform ivec2 screenSize;
uniform float gamma;

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

vec3 worldPos;
vec3 normal;
vec3 fragmentToEye;

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
    return p_F0 + (1.0 - p_F0) * pow(1.0 - p_cosTheta, 5.0);
} 

vec3 calcLightColor(vec3 p_albedoColor, vec3 p_normal, vec3 p_fragToEye, vec3 p_lightColor, vec3 p_lightDirection, float p_lightDistance, vec3 p_F0, float p_roughness, float p_metalic)
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
	return (kD * p_albedoColor / PI + specular) * radiance * NdotL;
}

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

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
	fragmentToEye = normalize(cameraPosVec - worldPos);
	
	// Extract roughness and metalness values
	float roughnessSqrt = matProperties.x;
	float metalic = matProperties.y;
	
	// Calculate F0, with minimum IOR as 0.04
	vec3 f0 = mix(vec3(0.04), diffuseColor, metalic);
	
	// Normalize direction light direction vector
	vec3 dirLightDirection =  normalize(directionalLight.m_direction);
	
	// Get dot product between the up vector (perpendicular to the ground) and directional light direction
	float dirLightFactor = dot(dirLightDirection, vec3(0.0, 1.0, 0.0)) + 0.001;
	
	// Initialize final color variable of this fragment
	vec3 finalLightColor = vec3(0.0);
	
	// Calculate directional light only if it is pointing from above the horizon
	if(dirLightFactor > 0.0)
	{
		float dirLightFactorSqrt = sqrt(dirLightFactor);
		vec3 dirLightColor = mix(g_sunSetColor, g_sunNoonColor, dirLightFactorSqrt);
		float dirLightIntensity = directionalLight.m_intensity * mix(g_sunSetIntensityMod, g_sunNoonIntensityMod, dirLightFactorSqrt);
	
		// Add ambient lighting
		finalLightColor += diffuseColor * dirLightIntensity * 0.003;
	
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
			metalic) * dirLightIntensity;// * min(1.0, (dirLightFactor /* 100.0*/) + 0.02);
	}
		
	for(int i = 0; i < numPointLights; i++)
	{		
		// Get light direction, extract length from it and normalize for usage as direction vector
		vec3 lightDirection =  pointLights[i].m_position - worldPos;
		float lightDistance = length(lightDirection);
		lightDirection = normalize(lightDirection);
		
		// Light color multiplied by intensity and divided by attenuation
		finalLightColor += (calcLightColor(diffuseColor, normal, fragmentToEye, pointLights[i].m_color, lightDirection, lightDistance, f0, roughnessSqrt, metalic) * pointLights[i].m_intensity);
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
			vec3 lightColor = (calcLightColor(diffuseColor, normal, fragmentToEye, spotLights[i].m_color, lightDirection, lightDistance, f0, roughnessSqrt, metalic) * spotLights[i].m_intensity);
			
			// Light restriction from cone
			float coneAttenuation = (1.0 - (1.0 - spotLightFactor) * 1.0 / (1.0 - spotLights[i].m_cutoffAngle));
			
			finalLightColor += lightColor * coneAttenuation;
		}
	}
	
	colorBuffer = vec4(finalLightColor + emissiveColor, 1.0);
	//colorBuffer = vec4(diffuseColor + emissiveColor, 1.0);
}