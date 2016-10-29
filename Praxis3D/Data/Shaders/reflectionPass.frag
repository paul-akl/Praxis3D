#version 450 core

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10
#define PI 3.1415926535

layout(location = 0) out vec4 emissiveBuffer;
layout(location = 1) out vec3 finalBuffer;

uniform sampler2D positionMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D matPropertiesMap;

uniform samplerCube staticEnvMap;

uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform vec3 cameraPosVec;
uniform ivec2 screenSize;

vec3 worldPos;
vec3 normal;
vec3 fragmentToEye;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

float somestep(float t) 
{
    return pow(t,4.0);
}

vec3 textureAVG(samplerCube tex, vec3 tc) 
{
    const float diff0 = 0.35;
    const float diff1 = 0.12;
 	vec3 s0 = texture(tex,tc).xyz;
    vec3 s1 = texture(tex,tc+vec3(diff0)).xyz;
    vec3 s2 = texture(tex,tc+vec3(-diff0)).xyz;
    vec3 s3 = texture(tex,tc+vec3(-diff0,diff0,-diff0)).xyz;
    vec3 s4 = texture(tex,tc+vec3(diff0,-diff0,diff0)).xyz;
    
    vec3 s5 = texture(tex,tc+vec3(diff1)).xyz;
    vec3 s6 = texture(tex,tc+vec3(-diff1)).xyz;
    vec3 s7 = texture(tex,tc+vec3(-diff1,diff1,-diff1)).xyz;
    vec3 s8 = texture(tex,tc+vec3(diff1,-diff1,diff1)).xyz;
    
    return pow((s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) * 0.111111111, vec3(2.2));;
}

vec3 textureBlured(samplerCube tex, vec3 tc) 
{
   	vec3 r = textureAVG(staticEnvMap, vec3(1.0,0.0,0.0));
    vec3 t = textureAVG(staticEnvMap, vec3(0.0,1.0,0.0));
    vec3 f = textureAVG(staticEnvMap, vec3(0.0,0.0,1.0));
    vec3 l = textureAVG(staticEnvMap, vec3(-1.0,0.0,0.0));
    vec3 b = textureAVG(staticEnvMap, vec3(0.0,-1.0,0.0));
    vec3 a = textureAVG(staticEnvMap, vec3(0.0,0.0,-1.0));
        
    float kr = dot(tc,vec3(1.0,0.0,0.0)) * 0.5 + 0.5; 
    float kt = dot(tc,vec3(0.0,1.0,0.0)) * 0.5 + 0.5;
    float kf = dot(tc,vec3(0.0,0.0,1.0)) * 0.5 + 0.5;
    float kl = 1.0 - kr;
    float kb = 1.0 - kt;
    float ka = 1.0 - kf;
    
    kr = somestep(kr);
    kt = somestep(kt);
    kf = somestep(kf);
    kl = somestep(kl);
    kb = somestep(kb);
    ka = somestep(ka);    
    
    float d;
    vec3 ret;
    ret  = f * kf; d  = kf;
    ret += a * ka; d += ka;
    ret += l * kl; d += kl;
    ret += r * kr; d += kr;
    ret += t * kt; d += kt;
    ret += b * kb; d += kb;
    
    return ret / d;
}

vec3 getColor(vec3 p_normal, vec3 p_viewDirection, float p_metalness, float p_roughness, float p_Fresnel) 
{
	// material
	float metallic = p_metalness;
	float roughness = p_roughness;//sin(iGlobalTime*0.4) * 0.5 + 0.5;
	float fresnel_pow = p_Fresnel;//0.5;// / (0.5 + roughness);
	//const vec3 color_mod = vec3(1.000, 0.766, 0.336);
	vec3 color_mod = vec3(1.0);
	//vec3 light_color = texture(staticEnvMap,vec3(1.0,0.0,0.0)).xyz * 1.2;
			
	//vec3 point = ray * dist;
	//vec3 normal = point - obj_pos;
	//normal = normalize(normal);
			
	// IBL
	vec3 ibl_diffuse = textureBlured(staticEnvMap, p_normal);
	vec3 ibl_reflection = textureBlured(staticEnvMap, reflect(p_viewDirection,p_normal));
	
	// fresnel
	float fresnel = max(1.0 - dot(p_normal,-p_viewDirection), 0.0);
	fresnel = pow(fresnel,fresnel_pow);    
	
	// reflection        
	vec3 refl = pow(texture(staticEnvMap,reflect(p_viewDirection,p_normal)).xyz, vec3(2.2));
	refl = mix(refl, ibl_reflection, (1.0-fresnel) * roughness);
	refl = mix(refl, ibl_reflection, roughness);
	
	// specular
	//vec3 light = normalize(vec3(-0.5,1.0,0.0));
	//float power = 1.0 / max(roughness * 0.4,0.01);
	//vec3 spec = light_color * phong(light,ray,normal,power);
	//refl -= spec;
	
	// diffuse
	vec3 diff = ibl_diffuse;
	diff = mix(diff, refl, fresnel);        

	vec3 color = mix(diff, refl /* color_mod*/, metallic);// + spec;
	return color;
}

void main(void) 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Get diffuse color (full-bright) from diffuse buffer and gamma-correct it
	vec3 diffuseColor = pow(texture(diffuseMap, texCoord).xyz, vec3(2.2));
	// Get pixel's position in world space
	vec3 worldPos = texture(positionMap, texCoord).xyz;
	// Get normal (in world space) and normalize it to minimize floating point approximation errors
	vec3 normal = normalize(texture(normalMap, texCoord).xyz);
	// Get material properties
	vec4 matProperties = texture(matPropertiesMap, texCoord).xyzw;
	
	// Extract roughness and metalness values
	float roughnessVar = matProperties.x;
	float metalness = matProperties.y;
	
	// Calculate the distance between camera and fragment
	fragmentToEye = cameraPosVec - worldPos;
	float distanceToFrag = length(fragmentToEye);
	
	// Calculate view direction (fragment to eye vector)
	fragmentToEye = normalize(fragmentToEye);
	
    vec3 I = normalize(worldPos - cameraPosVec);
    vec3 R = normalize(reflect(I, normal));
	
	float ior = mix(1.5, 40.5, metalness);
	//ior = 0.05;
	
	float F0 = abs((1.0 - ior) / (1.0 + ior));
	F0 = F0 * F0;
	vec3 F0vec = mix(vec3(F0), diffuseColor, metalness);
	
    vec3 halfVector = normalize(I + R);
    float dotLH = clamp(dot(R, halfVector), 0.0, 1.0);
    float dotLH5 = pow(1.0f - dotLH, 5);
	float F = F0 + (1.0f - F0) * (dotLH5);
	
	// Fresnel
    float NdotV = clamp(dot(normal, fragmentToEye), 0.0, 1.0);
	NdotV = pow(1.0 - NdotV, 5.0);
	float Fresnel = metalness + (1.0 - metalness) * (NdotV);
	
	
	float roughnessSqrt = roughnessVar * roughnessVar;
	
	
	vec3 envColor;// = pow(textureLod(staticEnvMap, R, 0).xyz, vec3(2.2));
	
	
	envColor = getColor(normal, I, metalness, roughnessVar, 3.0);
	
	//envColor = mix(pow(textureLod(staticEnvMap, R, 0).xyz, vec3(2.2)), pow(textureLod(staticEnvMap, R, 10).xyz, vec3(2.2)), 0.0);
	
	// Multiply the diffuse color by the amount of light the current pixel receives and gamma correct it
	finalBuffer = envColor * diffuseColor;
	//finalBuffer = vec3(0.0, 1.0, 0.0);
	//finalBuffer = pow(diffuseColor, vec3(1.0 / 2.2));
}