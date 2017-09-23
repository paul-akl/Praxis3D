#version 430 core

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10
#define PI 3.1415926535

layout(location = 0) out vec4 emissiveBuffer;
layout(location = 1) out vec3 finalBuffer;

//in vec2 texCoord;

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
uniform sampler2D matPropertiesMap;

uniform samplerCube staticEnvMap;

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

float saturate(float p_value)
{
	return clamp(p_value, 0.0f, 1.0f);
}

float G1V(float p_dotNV, float p_k)
{
    return 1.0f / (p_dotNV * (1.0f - p_k) + p_k);
}

/*float MicroFacetDistr_GGX(vec3 p_normal, vec3 p_halfVec, float p_roughnessSqrt)
{
	float NdotH = dot(p_normal, p_halfVec);
	if(NdotH > 0.0)
	{
		float NdotHSqrt = NdotH * NdotH;
		float microfacetDstrb = NdotHSqrt * p_roughnessSqrt + (1.0 - NdotHSqrt);
		return (NdotH * p_roughnessSqrt) / (3.14 * microfacetDstrb * microfacetDstrb);
	}
	else
		return 0.0;
}

float GeometryAtten_GGX(vec3 p_fragToEye, vec3 p_normal, vec3 p_halfVec, float p_roughnessSqrt)
{
    float VdotH = clamp(dot(p_fragToEye, p_halfVec), 0.0, 1.0);
    float VdotN = clamp(dot(p_fragToEye, p_normal), 0.0, 1.0);
	
	float geoFactor = (VdotH / VdotN);
	if(geoFactor > 0.0)
	{
		float VdotHSqrt = VdotH * VdotH;
		float geoAtt = (1.0 - VdotHSqrt) / VdotHSqrt;
		return 2.0 / (1.0 + sqrt(1.0 + p_roughnessSqrt * geoAtt));
	}
	else
		return 0.0;
}

vec3 Fresnel_Schlick(float p_cosT, vec3 p_F0)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - p_cosT, 5.0);
}*/

vec3 computePBRLighting(vec3 lightPos, vec3 lightColor, vec3 position, vec3 N, vec3 V, vec3 albedo, float roughness, vec3 F0) 
{

	float alpha = roughness*roughness;
	vec3 L = normalize(lightPos - position);
	vec3 H = normalize (V + L);

	float dotNL = clamp (dot (N, L), 0.0, 1.0);
	float dotNV = clamp (dot (N, V), 0.0, 1.0);
	float dotNH = clamp (dot (N, H), 0.0, 1.0);
	float dotLH = clamp (dot (L, H), 0.0, 1.0);

	float D, vis;
	vec3 F;

	// NDF : GGX
	float alphaSqr = alpha*alpha;
	float pi = 3.1415926535;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0;
	D = alphaSqr / (pi * denom * denom);

	// Fresnel (Schlick)
	float dotLH5 = pow (1.0 - dotLH, 5.0);
	F = F0 + (1.0 - F0)*(dotLH5);

	// Visibility term (G) : Smith with Schlick's approximation
	float k = alpha / 2.0;
	vis = G1V (dotNL, k) * G1V (dotNV, k);

	vec3 specular = /*dotNL **/ D * F * vis;

	vec3 ambient = vec3(.01);

	float invPi = 0.31830988618;
	vec3 diffuse = (albedo * invPi);


	return (diffuse + specular) * lightColor * dotNL ;
}

vec3 LightingFuncGGX_REF(vec3 p_normal, vec3 p_viewDir, vec3 p_lightDir, float p_roughnessSqrt, vec3 p_F0)
{
	// Calculate half vector between view and light directions
    vec3 halfVector = normalize(p_viewDir + p_lightDir);
	
	// Calculate required dot products
    float dotNL = saturate(dot(p_normal, p_lightDir));
    float dotNV = saturate(dot(p_normal, p_viewDir));
    float dotNH = saturate(dot(p_normal, halfVector));
    float dotLH = saturate(dot(p_lightDir, halfVector));

	// Calculate microfacet distributions (based on roughness)
	// Using GGX normal distribution function
    float RoughnessPow4 = p_roughnessSqrt * p_roughnessSqrt;
    float denom = dotNH * dotNH * (RoughnessPow4 - 1.0) + 1.0f;
    float D = RoughnessPow4 / (PI * denom * denom);

	// Calculate fresnel effect 
	// Using Schlick's approximation
    float dotLH5 = pow(1.0f - dotLH, 5);
	vec3 F = p_F0 + (vec3(1.0f) - p_F0) * (dotLH5);

	// Calculate geometric attenuation (or visibility term - self shadowing of microfacets)
	// Using Smith with Schlick's approximation
    float k2 = p_roughnessSqrt / 2.0f;
    float G = G1V(dotNL, k2) * G1V(dotNV, k2);
	
	// Multiple all the terms together
	return dotNL * D * F * G;
}

float SpecGGX(vec3 N, vec3 V, vec3 L, float roughness, float F0 )
{
	float SqrRoughness = roughness*roughness;

	vec3 H = normalize(V + L);

	float NdotL = clamp(dot(N,L),0.0,1.0);
	float NdotV = clamp(dot(N,V),0.0,1.0);
	float NdotH = clamp(dot(N,H),0.0,1.0);
	float LdotH = clamp(dot(L,H),0.0,1.0);

	float RoughnessPow4 = SqrRoughness * SqrRoughness;
	float denom = NdotH * NdotH * (RoughnessPow4 - 1.0) + 1.0;
	float D = RoughnessPow4 / (PI * denom * denom);

	float LdotH5 = 1.0 - LdotH;
    LdotH5 = LdotH5 * LdotH5 * LdotH5 * LdotH5 * LdotH5;
	float F = F0 + (1.0 - F0) * (LdotH5);

	float k = SqrRoughness/2.0;
	float G = G1V(NdotL, k) * G1V(NdotV, k);

	float specular = NdotL * D * F * G;
    
	return specular;
}

vec3 calcLightColor(vec3 p_normal, vec3 p_fragToEye, vec3 p_lightColor, vec3 p_lightDirection, vec3 p_F0, float p_diffuseAmount, float p_roughnessSqrt)
{	
	// Get specular and diffuse lighting
	vec3 specularColor = LightingFuncGGX_REF(p_normal, p_fragToEye, p_lightDirection, p_roughnessSqrt, p_F0);
    vec3 diffuseColor = vec3(clamp(dot(p_normal, p_lightDirection), 0.0, 1.0));
	
	// Add specular and diffuse together, and multiply it by the color of the light
	return (specularColor + diffuseColor * p_diffuseAmount) * p_lightColor;
}

/*vec3 calcLightColorOld(vec3 p_lightColor, vec3 p_lightDirection)
{
    // Get angle between normal and light direction
    //float NdotL = max(dot(normal, -p_lightDirection), 0.0);
	float NdotL = clamp( dot( normal, -p_lightDirection ), 0.0, 1.0 );
    
    float specular = 0.0;
    if(NdotL > 0.0)
    {
        // Calculate neccessary values
        vec3 halfVector = normalize(fragmentToEye - p_lightDirection );
		
		float NdotH = clamp( dot( normal, halfVector ), 0.0, 1.0 );
		float NdotV = clamp( dot( normal, fragmentToEye ), 0.0, 1.0 );
		float VdotH = clamp( dot( fragmentToEye, halfVector ), 0.0, 1.0 );
		        
        // Calculate geometric attenuation (self shadowing of microfacets)
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));
     	
		// Calculate microfacet distributions (roughness)
		// Using Beckmann distribution function
        float r1 = 1.0 / ( 4.0 * roughnessSqrt * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (roughnessSqrt * NdotH * NdotH);
        float microfacetDstrb = r1 * exp(r2);
        
		// Calculate fresnel effect (using Schlick's approximation)
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
		// Calculate specular component
        specular = max((fresnel * geoAtt * microfacetDstrb) / (NdotV * NdotL * 3.14), 0.0);
        //specular = (fresnel * geoAtt * microfacetDstrb) / (NdotV * NdotL * 3.14);
		
		//F0:"NdotL * (cSpecular * Rs + cDiffuse * (1-f0))"
		
		// Combine specular and diffuse components
		return p_lightColor * NdotL * (k + specular * (1.0 - k));
		//return p_lightColor * NdotL * specular;//(k + specular * (1.0 - k));
    }
	else
	{
		return vec3(0.0, 0.0, 0.0);
	}
}*/

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void) 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Get diffuse color (full-bright) from diffuse buffer and convert it to linear space
	vec3 diffuseColor = pow(texture(diffuseMap, texCoord).xyz, vec3(gamma));
	// Get pixel's position in world space
	vec3 worldPos = texture(positionMap, texCoord).xyz;
	// Get normal (in world space) and normalize it to minimize floating point approximation errors
	vec3 normal = normalize(texture(normalMap, texCoord).xyz);
	// Get material properties
	vec4 matProperties = texture(matPropertiesMap, texCoord).xyzw;
	
	// Extract roughness and metalness values
	float roughnessVar = matProperties.x;
	float metalness = matProperties.y;
	
	// Calculate view direction (fragment to eye vector)
	fragmentToEye = normalize(cameraPosVec - worldPos);
	
	float ior = mix(1.5, 40.5, metalness);
	ior = 40.5;
	
	float F0 = abs((1.0 - ior) / (1.0 + ior));
	F0 = F0 * F0;
	vec3 F0vec = mix(vec3(F0), diffuseColor, metalness);
		
	// Fresnel for the diffuse color
    float NdotV = clamp(dot(normal, fragmentToEye), 0.0, 1.0);
	NdotV = pow(1.0 - NdotV, 5.0);
	float diffuseAmount = 1.0 - (metalness + (1.0 - metalness) * (NdotV));
	
	float roughnessSqrt = roughnessVar * roughnessVar;
	
	// ior = from 1.2 to 10.0
	
	// Declare final color of the fragment and add directional light to it
	vec3 finalLightColor = calcLightColor(normal, fragmentToEye, directionalLight.m_color, normalize(-directionalLight.m_direction), F0vec, diffuseAmount, roughnessSqrt) * directionalLight.m_intensity;
	
	//vec3 finalLightColor = calcLightColor(normal, fragmentToEye, vec3(1.0, 1.0, 1.0), normalize(vec3(8.0, 10.0, 5.0)), F0vec, diffuseAmount, roughnessSqrt) * 1.0;
	
	for(int i = 0; i < numPointLights; i++)
	{		
		// Get light direction, extract length from it and normalize for usage as direction vector
		vec3 lightDirection =  pointLights[i].m_position - worldPos;
		float lightDistance = length(lightDirection);
		lightDirection = normalize(lightDirection);
		
		// Add up constant, linear and quadratic attenuation
		float attenuation = pointLights[i].m_attenConstant + 
							pointLights[i].m_attenLinear * lightDistance + 
							pointLights[i].m_attenQuad * lightDistance * lightDistance;
							
		// Light color multiplied by intensity and divided by attenuation
		finalLightColor += (calcLightColor(normal, fragmentToEye, pointLights[i].m_color, lightDirection, F0vec, diffuseAmount, roughnessSqrt) * pointLights[i].m_intensity) / attenuation;
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
			
			// Add up constant, linear and quadratic attenuation
			float attenuation = spotLights[i].m_attenConstant + 
								spotLights[i].m_attenLinear * lightDistance + 
								spotLights[i].m_attenQuad * lightDistance * lightDistance;
			
			// Light color multiplied by intensity
			vec3 lightColor = (calcLightColor(normal, fragmentToEye, spotLights[i].m_color, lightDirection, F0vec, diffuseAmount, roughnessSqrt) * spotLights[i].m_intensity);
			
			// Light restriction from cone
			float coneAttenuation = (1.0 - (1.0 - spotLightFactor) * 1.0 / (1.0 - spotLights[i].m_cutoffAngle));
			
			finalLightColor += (lightColor / attenuation) * coneAttenuation;
		}
	}
	emissiveBuffer = vec4(1.0, 0.0, 0.0, 0.0);
	
	// Multiply the diffuse color by the amount of light the current pixel receives and gamma correct it
	
	finalBuffer = diffuseColor * finalLightColor;//texture(diffuseMap, texCoord).xyz;
	//finalBuffer = vec3(0.0, 1.0, 0.0);//diffuseColor * finalLightColor;
	//finalBuffer = simpleToneMapping(diffuseColor * finalLightColor, 2.2);
	//finalBuffer = vec3(roughnessVar, roughnessVar, roughnessVar);
	//finalBuffer = vec4(texture(staticEnvMap, vec3(0.0, 0.0, 0.0)).xyz, 1.0);
	//finalBuffer = vec4(metallic, metallic, metallic, 1.0);
	//finalBuffer = vec4(pow(mix(diffuseColor, vec3(0.0, 0.0, 0.0), 1.0 - metallic) * finalLightColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(pow(finalLightColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(pow(diffuseColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(texture(normalMap, texCoord).xyz, 1.0);
	//finalBuffer = vec4(roughnessVar, roughnessVar, roughnessVar, 1.0);
}