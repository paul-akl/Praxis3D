#version 330

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10

layout(location = 0) out vec4 emissiveBuffer;
layout(location = 1) out vec4 finalBuffer;

//in vec2 texCoord;

uniform sampler2D positionMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform ivec2 screenSize;

uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform vec3 cameraPosVec;

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

uniform DirectionalLight directionalLight;
	
vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
    //return gl_FragCoord.xy / vec2(1600, 900);
}

void main(void) 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	vec3 varNormal = texture(normalMap, texCoord).xyz;
	
	//vec3 varEyeDir = vec3(viewMat[2].xyz);
	vec3 worldPos = texture(positionMap, texCoord).xyz;
	
//vec3 varEyeDir = normalize(worldPos - cameraPosVec);
	vec3 varEyeDir = normalize(cameraPosVec - worldPos);
	//vec3 varEyeDir = vec3(viewMat[3].xyz);
	//vec3 varEyeDir = vec4(vec4(1.0, 0.0, 0.0, 1.0) * viewMat).xyz;
	
	//vec3 lightDirection = normalize(vec3(0.5, 0.0, 0.5));
	//vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.5));
	vec3 lightDirection = directionalLight.m_direction;
	
	// set important material values
    float roughnessValue = 0.3; // 0 : smooth, 1: rough
    float F0 = 1.8; // fresnel reflectance at normal incidence
    float k = 0.01; // fraction of diffuse reflection (specular reflection = 1 - k)
    //vec3 lightColor = directionalLight.m_color;
	vec3 lightColor = spotLights[1].m_color;
    
    // interpolating normals will change the length of the normal, so renormalize the normal.
    vec3 normal = normalize(varNormal);
    
    // do the lighting calculation for each fragment.
    float NdotL = max(dot(normal, lightDirection), 0.0);
    
    float specular = 0.0;
    if(NdotL > 0.0)
    {
        vec3 eyeDir = normalize(varEyeDir);

        // calculate intermediary values
        vec3 halfVector = normalize(lightDirection + eyeDir);
        float NdotH = max(dot(normal, halfVector), 0.0); 
        float NdotV = max(dot(normal, eyeDir), 0.0); // note: this could also be NdotL, which is the same value
        float VdotH = max(dot(eyeDir, halfVector), 0.0);
        float mSquared = roughnessValue * roughnessValue;
        
        // geometric attenuation
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));
     
        // roughness (or: microfacet distribution function)
        // beckmann distribution function
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);
        
        // fresnel
        // Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
        specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14);
    }

	vec3 color = pow(texture(diffuseMap, texCoord).xyz, vec3(2.2));
    vec3 finalValue = lightColor * NdotL * (k + specular * (1.0 - k));
	
	finalBuffer = vec4(pow(color * finalValue, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(directionalLight.m_color, 1.0);
	//finalBuffer = vec4(normalize(lightDirection + varEyeDir), 1.0);

	// Get diffuse color (full-bright) from diffuse buffer
	//vec3 color = texture(diffuseMap, texCoord).xyz;

	//finalBuffer = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}