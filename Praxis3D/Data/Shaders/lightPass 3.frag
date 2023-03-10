#version 430 core

#define MAX_NUM_POINT_LIGHTS 20
#define MAX_NUM_SPOT_LIGHTS 10

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
float roughnessSqrt;

// set important material values
float roughnessVar = 0.1; // 0.1	// 0 : smooth, 1: rough
//float roughnessVar = 0.8;
float F0 = 0.02;//0.171968833;//1.0; 			// fresnel reflectance at normal incidence
vec3 F0vec;
//float k = 0.01; 				// fraction of diffuse reflection (specular reflection = 1 - k)
float k = 0.8; //0.5
float metallic;
vec3 envColor;

float ref_at_norm_incidence = F0;
vec3 viewer;

float saturate(float p_value)
{
	return clamp(p_value, 0.0f, 1.0f);
}

float G1V(float p_dotNV, float p_k)
{
    return 1.0f / (p_dotNV * (1.0f - p_k) + p_k);
}

float MicroFacetDistr_GGX(vec3 p_normal, vec3 p_halfVec, float p_roughnessSqrt)
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
}

vec3 calcLightColor2(vec3 p_lightColor, vec3 p_lightDirection)
{
	//float NoV = saturate( dot(normal, V) );
	float NoV = abs( dot(normal, fragmentToEye) ) + 0.00001;//1e-5;

	// Diffuse_Lambert
	//Shared.DiffuseMul = DiffuseColor * (1.0 / 3.14);

	// D_GGX, Vis_SmithJointApprox
	float m = roughnessVar * roughnessVar;
	float m2 = m * m;
	float SpecularMul = (0.5 / 3.14) * m2;
	float VisMad = ( 2 * NoV * ( 1 - m ) + m, NoV * m );
	
	// F_Schlick
	//SpecularMul *= saturate( 50.0 * k );
	return vec3(1.0);
}

vec3 calcLightColor3(vec3 p_lightColor, vec3 p_lightDirection)
{
	// Compute any aliases and intermediary values
    // -------------------------------------------
	
	vec3 half_vector = normalize(fragmentToEye - p_lightDirection );
    float NdotL        = clamp( dot( normal, -p_lightDirection ), 0.0, 1.0 );
    float NdotH        = clamp( dot( normal, half_vector ), 0.0, 1.0 );
    float NdotV        = clamp( dot( normal, viewer ), 0.0, 1.0 );
    float VdotH        = clamp( dot( viewer, half_vector ), 0.0, 1.0 );
    float r_sq         = roughnessVar * roughnessVar;
 
    // Evaluate the geometric term
    // --------------------------------
    float geo_numerator   = 2.0f * NdotH;
    float geo_denominator = VdotH;
 
    float geo_b = (geo_numerator * NdotV ) / geo_denominator;
    float geo_c = (geo_numerator * NdotL ) / geo_denominator;
    float geo   = min( 1.0f, min( geo_b, geo_c ) );
 
    // Now evaluate the roughness term
    // -------------------------------
    float roughness;
    
	float roughness_a = 1.0f / ( 4.0f * r_sq * pow( NdotH, 4 ) );
	float roughness_b = NdotH * NdotH - 1.0f;
	float roughness_c = r_sq * NdotH * NdotH;
	roughness = roughness_a * exp( roughness_b / roughness_c );
 
    // Next evaluate the Fresnel value
    // -------------------------------
    float fresnel = pow( 1.0f - VdotH, 5.0f );
    fresnel *= ( 1.0f - F0 );
    fresnel += F0;
 
    // Put all the terms together to compute
    // the specular term in the equation
    // -------------------------------------
    float Rs_numerator   = ( fresnel * geo * roughness );
    float Rs_denominator  = NdotV * NdotL;
    float Rs             = Rs_numerator/ Rs_denominator;
  
    float specular = (fresnel * geo * roughness) / (NdotV * NdotL);
    // Put all the parts together to generate
    // the final colour
    // --------------------------------------
	
	return p_lightColor * NdotL * specular;
	
    //float3 final = max(0.0f, NdotL) * (cSpecular * Rs + cDiffuse);
 
    // Return the result
    // -----------------
    //return float4( final, 1.0f );
}

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

/*float SpecGGX(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
	float SqrRoughness = roughness*roughness;

	vec3 H = normalize(V+L);

	float NdotL = clamp(dot(N,L),0.0,1.0);
	float NdotV = clamp(dot(N,V),0.0,1.0);
	float NdotH = clamp(dot(N,H),0.0,1.0);
	float LdotH = clamp(dot(L,H),0.0,1.0);

	// Geom term
	float RoughnessPow4 = SqrRoughness*SqrRoughness;
	float pi = 3.14159;
	float denom = NdotH * NdotH *(RoughnessPow4-1.0) + 1.0;
	float D = RoughnessPow4/(pi * denom * denom);

	// Fresnel term 
	float LdotH5 = 1.0-LdotH;
    LdotH5 = LdotH5*LdotH5*LdotH5*LdotH5*LdotH5;
	float F = F0 + (1.0-F0)*(LdotH5);

	// Vis term 
	float k = SqrRoughness/2.0;
	float Vis = G1V(NdotL,k)*G1V(NdotV,k);

	float specular = NdotL * D * F * Vis;
    
	return specular;
}*/

vec3 LightingFuncGGX_REF(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
{
    float alpha = roughness;//roughness*roughness;

    vec3 H = normalize(V+L);

    float dotNL = saturate(dot(N,L));
    float dotNV = saturate(dot(N,V));
    float dotNH = saturate(dot(N,H));
    float dotLH = saturate(dot(L,H));

    //float F, D, vis;
	float D, vis;
	vec3 F;

    // D
    float alphaSqr = alpha*alpha;
    float pi = 3.14159f;
    float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
    D = alphaSqr/(pi * denom * denom);

    // F
    float dotLH5 = pow(1.0f - dotLH, 5);
	F = F0 + (vec3(1.0f) - F0) * (dotLH5);
    //F = F0 + (1.0-F0)*(dotLH5);

    // V
    float k2 = alpha / 2.0f;
    vis = G1V(dotNL, k2) * G1V(dotNV, k2);
	
	float metallic2 = (1.0f - metallic);// / 2.0f;
	
	vec3 specular;
	
	//if(metallic == 1.0f)
	//{
		specular = dotNL * D * F * vis;
	//}
	//else
	//{
		//specular = (dotNL) * (metallic2 + D * F * vis * (1.0 - metallic2));
	//}
	
    //vec3 specular = (dotNL) * (metallic2 + D * F * vis * (1.0 - metallic2));
	
	//specular = ((1.0 - dotNL) * (1.0 - metallic2)) + D * F * vis;
	//specular = ((1.0 - dotNL) * (1.0 - metallic2)) + (metallic2 + D * F * vis * (1.0 - metallic2));
	
    return specular;
}

float SpecGGX(vec3 N, vec3 V, vec3 L, float roughness, float F0 )
{
	float SqrRoughness = roughness*roughness;

	vec3 H = normalize(V+L);

	float NdotL = clamp(dot(N,L),0.0,1.0);
	float NdotV = clamp(dot(N,V),0.0,1.0);
	float NdotH = clamp(dot(N,H),0.0,1.0);
	float LdotH = clamp(dot(L,H),0.0,1.0);

	// Geom term
	float RoughnessPow4 = SqrRoughness*SqrRoughness;
	float pi = 3.14159;
	float denom = NdotH * NdotH *(RoughnessPow4-1.0) + 1.0;
	float D = RoughnessPow4/(pi * denom * denom);

	// Fresnel term 
	float LdotH5 = 1.0-LdotH;
    LdotH5 = LdotH5*LdotH5*LdotH5*LdotH5*LdotH5;
	float F = F0 + (1.0-F0)*(LdotH5);

	// Vis term 
	float k = SqrRoughness/2.0;
	float Vis = G1V(NdotL,k)*G1V(NdotV,k);

	float specular = NdotL * D * F * Vis;
    
	return specular;
}

vec3 calcLight(vec3 p_normal, vec3 p_fragToEye, vec3 p_lightColor, vec3 p_lightDirection, vec3 p_F0, float p_roughnessSqrt)
{	
	vec3 lightDir = -p_lightDirection;
		
	//float spec = SpecGGX(p_normal, p_fragToEye, lightDir, roughnessVar, F0);
	vec3 spec = LightingFuncGGX_REF(p_normal, p_fragToEye, lightDir, roughnessSqrt, F0vec);
	
    float dif = dot(p_normal, lightDir);
		
	// Fresnel
    float NdotV = clamp(dot(p_normal, p_fragToEye), 0.0, 1.0);
	NdotV = pow(1.0 - NdotV, 5.0);
	float Fresnel = metallic + (1.0 - metallic) * (NdotV);

    // Tint lights
    vec3 SpecColor = spec * p_lightColor;
    vec3 DiffColor = dif * p_lightColor * (1.0 - Fresnel);
	
    vec3 lightSum = max(((DiffColor + SpecColor)), vec3(0.0, 0.0, 0.0));
    
	// Fresnel 2
    /*vec3 H = normalize(fragmentToEye + lightDir);
    float dotLH = saturate(dot(lightDir,H));
    float dotLH5 = pow(1.0f - dotLH, 5);
	float F = F0 + (1.0 - F0) * (dotLH5);
	F = metallic + (1.0 - metallic) * F;*/
	
    // Add GI
    //const float	cAmbientMin = 0.04;    
    //float		ambient = cAmbientMin * (IsInSphere);    
    //vec3		ColorAmbient = vec3(ambient,ambient,ambient);
    //vec3		GIReflexion = GetGIReflexion ( normal, roughness );
    
    
    //ColorAmbient = GIReflexion * cAmbientMin;
        
    //vec3 lightSum = max(((DiffColor + SpecColor) * (1.0 - cAmbientMin)), vec3(0.0, 0.0, 0.0));
    //return ( lightSum + ColorAmbient + ( Fresnel * GIReflexion ) ) * IsInSphere;
	
    return lightSum;
	
	/*

    float dotNL = saturate(dot(p_normal, -p_lightDirection));
	
	//spec = LightingFuncGGX_REF(p_normal, p_fragToEye, -p_lightDirection, p_roughnessSqrt, F0vec);
	
    vec3 H = normalize(p_fragToEye + (-p_lightDirection));
    float dotLH = saturate(dot(-p_lightDirection,H));
    float dotLH5 = pow(1.0f - dotLH, 5);
    //F = F0 + (1.0-F0)*(dotLH5);
	vec3 F = F0vec + (1.0f - F0vec) * (dotLH5);
	
	float metallic2 = 1.0 - metallic;
	
	float diffuse = (1.0 - dotNL) * (1.0 - metallic);
		//return p_lightColor * NdotL * (k + specular * (1.0 - k));
	
	//return p_lightColor * spec;*/

	/*/ Calculate the specular contribution
    float3 ks = 0;
    float3 specular = GGX_Specular(specularCubemap, normal, viewVector, roughness, F0, ks );
    float3 kd = (1 - ks) * (1 - metallic);
    // Calculate the diffuse contribution
    float3 irradiance = texCUBE(diffuseCubemap_Sampler, normal ).rgb;
    float3 diffuse = materialColour * irradiance;

    return float4( kd * diffuse + /*ks* / specular, 1);  */   
	
	//vec3 reflectionVec = reflect(-p_fragToEye, p_normal);
    /*vec3 radiance = vec3(0);
	
	//float NdotV = clamp(dot(p_normal, p_fragToEye), 0.0, 1.0);
	float NdotL = clamp(dot(p_normal, -p_lightDirection), 0.0, 1.0);
	
	//if(NdotL > 0.0)
	{
       // vec3 halfVector = normalize(p_lightDirection - p_fragToEye);
        vec3 halfVector = normalize(p_fragToEye + p_lightDirection);
		float NdotH = clamp(dot(p_normal, halfVector), 0.0, 1.0);
		float VdotH = clamp(dot(p_fragToEye, halfVector), 0.0, 1.0);
		
		float cosT = clamp(dot(p_lightDirection, p_normal), 0.0, 1.0); // NdotL
		float sinT = sqrt(1.0 - (cosT * cosT));
		
		vec3 fresnel = Fresnel_Schlick(VdotH, p_F0);
		float geometry = GeometryAtten_GGX(p_fragToEye, p_normal, halfVector, p_roughnessSqrt) * GeometryAtten_GGX(p_lightDirection, p_normal, halfVector, p_roughnessSqrt);
		//float geometry = GeometryAtten_GGX(p_lightDirection, p_normal, halfVector, p_roughnessSqrt);
		float denominator = clamp(4.0 * (NdotV * NdotH + 0.05), 0.0, 1.0);
		
		//kS += fresnel;
		
		radiance = p_lightColor * geometry * fresnel * sinT / denominator;
	}
	
	return radiance;*/
}

vec3 calcLightColor(vec3 p_lightColor, vec3 p_lightDirection)
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
		
		/*float NdotL = max(dot(normal, -p_lightDirection), 0.0);
        float NdotH = max(dot(normal, halfVector), 0.0); 
        float NdotV = max(dot(normal, fragmentToEye), 0.0);
		float VdotH = max(dot(fragmentToEye, halfVector), 0.0);*/
        
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
}

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
	return vec3(0.0);
    /*float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);

    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;*/
}

vec3 PrefilterEnvMap( float Roughness , vec3 R )
{
	return vec3(0.0);
    /*vec3 N = R;
    vec3 V = R;
    vec3 PrefilteredColor = vec3(0);
    float TotalWeight = 0.0000001f;
    const uint NumSamples = 1024;

    for( uint i = 0; i < NumSamples; i++ )
    {
        vec2 Xi = vec2(RandomNumBuffer[i * 2 + 0], RandomNumBuffer[i * 2 + 1]);
        vec3 H = ImportanceSampleGGX( Xi, Roughness , N );
        vec3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( dot( N, L ) );

        if( NoL > 0 )
        {
            PrefilteredColor += EnvMap.SampleLevel( linearSampler, L, 0 ).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;*/
}

vec3 S(vec2 p_vec)
{
	return vec3(1.0);
}

 float F(float VoH)
 {
	return 1.0f;
 }
 float G(float NoL, float NoV, float NoH, float VoH)
 {
	return 1.0f;
 }
	
// Hammersley function (return random low-discrepency points)
vec2 Hammersley(uint i, uint N)
{
  return vec2(
    float(i) / float(N),
    float(bitfieldReverse(i)) * 2.3283064365386963e-10
  );
}

// Computes the exact mip-map to reference for the specular contribution.
// Accessing the proper mip-map allows us to approximate the integral for this
// angle of incidence on the current object.
float compute_lod(uint NumSamples, float NoH)
{
	return 0.0;
	//float dist = D(NoH); // Defined elsewhere as subroutine
	//return 0.5 * (log2(float(Dimensions.x * Dimensions.y) / NumSamples) - log2(dist));
}
 
// Calculates the specular influence for a surface at the current fragment
// location. This is an approximation of the lighting integral itself.
vec3 radiance(vec3 N, vec3 V)
{
  // Precalculate rotation for +Z Hemisphere to microfacet normal.
  //vec3 UpVector = abs(N.z) < 0.999 ? ZAxis : XAxis;
  vec3 UpVector = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 TangentX = normalize(cross( UpVector, N ));
  vec3 TangentY = cross(N, TangentX);
 
  // Note: I ended up using abs() for situations where the normal is
  // facing a little away from the view to still accept the approximation.
  // I believe this is due to a separate issue with normal storage, so
  // you may only need to saturate() each dot value instead of abs().
  float NoV = abs(dot(N, V));
 
  // Approximate the integral for lighting contribution.
  vec3 fColor = vec3(0.0);
  const uint NumSamples = 20;
  for (uint i = 0; i < NumSamples; ++i)
  {
    vec2 Xi = Hammersley(i, NumSamples);
    vec3 Li = S(Xi); // Defined elsewhere as subroutine
    vec3 H  = normalize(Li.x * TangentX + Li.y * TangentY + Li.z * N);
    vec3 L  = normalize(-reflect(V, H));
 
    // Calculate dot products for BRDF
    float NoL = abs(dot(N, L));
    float NoH = abs(dot(N, H));
    float VoH = abs(dot(V, H));
    float lod = compute_lod(NumSamples, NoH);
 
    float F_ = F(VoH); // Defined elsewhere as subroutine
    float G_ = G(NoL, NoV, NoH, VoH); // Defined elsewhere as subroutine
    vec3 LColor = textureLod(staticEnvMap, L, lod).rgb;
 
    // Since the sample is skewed towards the Distribution, we don't need
    // to evaluate all of the factors for the lighting equation. Also note
    // that this function is calculating the specular portion, so we absolutely
    // do not add any more diffuse here.
    fColor += F_ * G_ * LColor * VoH / (NoH * NoV);
  }
 
  // Average the results
  return fColor / float(NumSamples);
}
 
vec3 main2()
{
  vec3 V = fragmentToEye;//normalize((-fragmentToEye * cameraPosVec).xyz);
  vec3 color;
 
  // No object, instead display the environment.
  /*if (depth() == 1.0)
  {
    color = textureLod(staticEnvMap, -V, 0.0).rgb;
  }*/
 
  // Object, approximate the ambient light.
  //else
  //{
    vec3 N = normalize((-fragmentToEye * normal).xyz);
    vec3 L = normalize(-reflect(V, N));
    float NoV = saturate(dot(N, V));
    float NoL = saturate(dot(N, L));
 
    // Calculate different portions of the color
    vec3 irrMap = textureLod(staticEnvMap, N, 0.0).rgb;
    vec3 Kdiff  = irrMap /** baseColor()*/ / 3.14159265358979323846;
    vec3 Kspec  = radiance(N, V);
 
    // Mix the materials
    color = Kdiff;//BlendMaterial(Kdiff, Kspec);
  //}
	return color;
  //fFragColor = vec4(color, 1.0);
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
	roughnessVar = matProperties.x;
	metallic = matProperties.y;
	
	// Calculate view direction (fragment to eye vector)
	fragmentToEye = normalize(cameraPosVec - worldPos);
	
	ref_at_norm_incidence = F0;
	viewer = fragmentToEye;
		
	//fragmentToEye = normalize(worldPos - cameraPosVec);
	//roughnessSqrt = roughnessVar;// * roughnessVar;

	//k = 1.0 - positionAndGloss.w;
	//roughnessVar = (normalAndSpecular.w + positionAndGloss.w) / 2.0;
	roughnessSqrt = roughnessVar * roughnessVar;
	
	// ior = from 1.2 to 10.0
	
	//envColor = pow(textureLod(staticEnvMap, R, 0).xyz, vec3(2.2));
	float ior = mix(1.5, 40.5, metallic);
	ior = 4.5;
	
	F0 = abs((1.0 - ior) / (1.0 + ior));
	F0 = F0 * F0;
	F0vec = vec3(F0);
	F0vec = mix(F0vec, diffuseColor, metallic);
	
	
	//vec3 lightDir = -p_lightDirection;

	//float spec = SpecGGX(p_normal, p_fragToEye, lightDir, roughnessVar, F0);
	//vec3 lightDir = -normalize(directionalLight.m_direction);
	//vec3 p_lightColor = vec3(1.0);
	
	//vec3 spec = SpecGGX(normal, fragmentToEye, R, roughnessVar, F0) * envColor;
	
    //float dif = dot(normal, lightDir);
	//float dif = dot(p_normal, -p_lightDirection);
	
	// Fresnel
    //float NdotV = clamp(dot(normal, fragmentToEye), 0.0, 1.0);
	//NdotV = pow(1.0 - NdotV, 5.0);    
	//float Fresnel = metallic + (1.0 - metallic) * (NdotV);

    //vec3 H = normalize(fragmentToEye + R);
    //float dotLH = saturate(dot(R,H));
    //float dotLH5 = pow(1.0f - dotLH, 5);
	//float F = F0 + (1.0 - F0) * (dotLH5);
	//F = metallic + (1.0 - metallic) * F;
	//vec3 F = F0vec + (vec3(1.0) - F0vec) * (dotLH5);
	
    // Tint lights
    //vec3 SpecColor = spec * p_lightColor;
    //vec3 DiffColor = dif * p_lightColor * (1.0 - Fresnel);
	
	
    //float dif = dot(normal, R);
    //vec3 DiffColor = dif * diffEnvColor * (1.0 - Fresnel);
	
	//envColor *= spec;
	
    // envColor = max(((/*diffuseColor*DiffColor +*/ envColor*spec)), vec3(0.0, 0.0, 0.0));
		
	// Fresnel
    float NdotV = clamp(dot(normal, fragmentToEye), 0.0, 1.0);
	NdotV = pow(1.0 - NdotV, 5.0);
	float Fresnel = metallic + (1.0 - metallic) * (NdotV);
	
    //vec3 lightSum = max(((DiffColor + SpecColor)), vec3(0.0, 0.0, 0.0));
	vec3 lightSum;
	
    vec3 I = normalize(worldPos - cameraPosVec);
    vec3 R = normalize(reflect(I, normal));
	
	
	vec3 diffEnvColor = (pow(textureLod(staticEnvMap, R, 9).xyz, vec3(2.2)) + 
						pow(textureLod(staticEnvMap, R, 10).xyz, vec3(2.2)) +
						pow(textureLod(staticEnvMap, R, 11).xyz, vec3(2.2))) / 3.0;
						
	int texLOD = 0;//int(10 * roughnessSqrt);
	envColor = mix(pow(textureLod(staticEnvMap, R, 0).xyz, vec3(2.2)), pow(textureLod(staticEnvMap, R, 10).xyz, vec3(2.2)), 0.0);
	
	vec3 spec = min(LightingFuncGGX_REF(normal, fragmentToEye, R, roughnessSqrt, diffuseColor), 1.0);
	
    vec3 SpecColor = spec * envColor;
    vec3 DiffColor = dot(normal, R) * diffEnvColor * (1.0 - Fresnel);
		
	vec3 Rnew = R;
	
	lightSum = main2();
	
	//Rnew = normalize(R + vec3(0.1, 0.0, 0.0));
	//lightSum = calcLight(normal, fragmentToEye, pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)), -R, vec3(F0), roughnessSqrt);
	//lightSum = pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(0.08, 0.0, 0.0));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(-0.08, 0.0, 0.0));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(0.0, 0.08, 0.0));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(0.0, -0.08, 0.0));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(0.0, 0.0, 0.08));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	//Rnew = normalize(R + vec3(0.0, 0.0, -0.08));
	//lightSum += pow(textureLod(staticEnvMap, Rnew, 0).xyz, vec3(2.2)) * LightingFuncGGX_REF(normal, -I, Rnew, roughnessSqrt, F0vec);
	
	//lightSum /= 6;
	
	//envColor += dif;
	
    //vec3 lightSum = max(((DiffColor + SpecColor)), vec3(0.0, 0.0, 0.0));
    //float dotLH5 = pow(1.0f - dotLH, 5);
    //F = F0 + (1.0-F0)*(dotLH5);
	//F = F0 + (1.0f - F0) * (dotLH5);
	
    // Tint lights
    //vec3 SpecColor = spec * p_lightColor;
    //vec3 DiffColor = dif * p_lightColor * (1.0 - Fresnel);
	//spec = max(((DiffColor + SpecColor)), vec3(0.0, 0.0, 0.0)) * directionalLight.m_intensity;
	
	//vec3 spec = LightingFuncGGX_REF(normal, fragmentToEye, -normalize(directionalLight.m_direction), roughnessSqrt, F0vec);
	
	//envColor = spec * pow(textureLod(staticEnvMap, R, 0).xyz, vec3(2.2));
	//envColor = pow(textureLod(staticEnvMap, R, 5).xyz, vec3(2.2));
	
	//envColor = mix(pow(textureLod(staticEnvMap, R, 5).xyz, vec3(2.2)), pow(textureLod(staticEnvMap, R, 6).xyz, vec3(2.2)), 0.5);
	
	//float spec = SpecGGX(p_normal, p_fragToEye, R, roughnessVar, F0);
	//envColor *= 
	
	//float reflectance = 0.5;
	//F0vec = 0.16 * reflectance * reflectance * (1.0 - metallic) + diffuseColor * metallic;
	
	//float roughness = 1.0 - smoothness*smoothness;
    //vec3 F0 = 0.16*reflectance*reflectance * (1.0-metalMask) + baseColor*metalMask;
    //vec3 albedo = baseColor;
	
	//float3 F0 = abs ((1.0 - ior) / (1.0 + ior));
	//F0 = F0 * F0;
	//F0 = lerp(F0, materialColour.rgb, metallic);
	
	//k = 1.0 - normalAndSpecular.w;
	//k = 0.2;
	//roughnessVar = 0.1;// - ((positionAndGloss.w + normalAndSpecular.w) / 2.0);
	
	
	// Declare final color of the fragment and add directional light to it
	vec3 finalLightColor = calcLight(normal, fragmentToEye, directionalLight.m_color, normalize(directionalLight.m_direction), vec3(F0), roughnessSqrt) * directionalLight.m_intensity;// + directionalLight.m_color * 0.005;
	//vec3 finalLightColor = vec3(0.0);
	
	for(int i = 0; i < numPointLights; i++)
	{		
		// Get light direction, extract length from it and normalize for usage as direction vector
		vec3 lightDirection = worldPos - pointLights[i].m_position;
		//vec3 lightDirection =  pointLights[i].m_position - worldPos;
		float lightDistance = length(lightDirection);
		lightDirection = normalize(lightDirection);
		
		// Add up constant, linear and quadratic attenuation
		float attenuation = pointLights[i].m_attenConstant + 
							pointLights[i].m_attenLinear * lightDistance + 
							pointLights[i].m_attenQuad * lightDistance * lightDistance;
						 
		// Light color multiplied by intensity and divided by attenuation
		
		//finalLightColor += (calcLightColor(pointLights[i].m_color, lightDirection) * pointLights[i].m_intensity) / attenuation;
		finalLightColor += (calcLight(normal, fragmentToEye, pointLights[i].m_color, lightDirection, vec3(F0), roughnessSqrt) * pointLights[i].m_intensity) / attenuation;
		//finalLightColor += (computePBRLighting(pointLights[i].m_position, pointLights[i].m_color, worldPos, normal, fragmentToEye, diffuseColor, roughnessVar, F0vec) * pointLights[i].m_intensity) / attenuation;
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
			vec3 lightDirection = worldPos - spotLights[i].m_position;
			//vec3 lightDirection =  spotLights[i].m_position - worldPos;
			float lightDistance = length(lightDirection);
			lightDirection = normalize(lightDirection);
			
			// Add up constant, linear and quadratic attenuation
			float attenuation = spotLights[i].m_attenConstant + 
								spotLights[i].m_attenLinear * lightDistance + 
								spotLights[i].m_attenQuad * lightDistance * lightDistance;
			
			// Light color multiplied by intensity
			//finalLightColor += (calcLight(normal, fragmentToEye, pointLights[i].m_color, lightDirection, vec3(F0), roughnessSqrt) * pointLights[i].m_intensity) / attenuation;
			vec3 lightColor = (calcLight(normal, fragmentToEye, spotLights[i].m_color, lightDirection, vec3(F0), roughnessSqrt) * spotLights[i].m_intensity);
			//vec3 lightColor = (calcLightColor(spotLights[i].m_color, lightDirection) * spotLights[i].m_intensity);
			
			// Light restriction from cone
			float coneAttenuation = (1.0 - (1.0 - spotLightFactor) * 1.0 / (1.0 - spotLights[i].m_cutoffAngle));
			
			finalLightColor += (lightColor / attenuation) * coneAttenuation;
		}
	}
		
	// Multiply the diffuse color by the amount of light the current pixel receives and gamma correct it
	finalBuffer = pow(diffuseColor * finalLightColor, vec3(1.0 / 2.2));
	//finalBuffer = vec3(roughnessVar, roughnessVar, roughnessVar);
	//finalBuffer = vec4(texture(staticEnvMap, vec3(0.0, 0.0, 0.0)).xyz, 1.0);
	//finalBuffer = vec4(metallic, metallic, metallic, 1.0);
	//finalBuffer = vec4(pow(mix(diffuseColor, vec3(0.0, 0.0, 0.0), 1.0 - metallic) * finalLightColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(pow(finalLightColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(pow(diffuseColor, vec3(1.0 / 2.2)), 1.0);
	//finalBuffer = vec4(texture(normalMap, texCoord).xyz, 1.0);
	//finalBuffer = vec4(roughnessVar, roughnessVar, roughnessVar, 1.0);
}