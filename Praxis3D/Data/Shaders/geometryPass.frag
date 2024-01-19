#version 430 core

#define STOCHASTIC_SAMPLING 0
#define STOCHASTIC_SAMPLING_MIPMAP_SEAM_FIX 1

#define PARALLAX_MAPPING 0
#define PARALLAX_MAPPING_METHOD 5

#define MIN_LOD_PARALLAX 0.1
#define MAX_LOD_PARALLAX 20.0
#define LOD_PARALLAX_THRESHOLD 0.0
#define PARALLAX_SCALE_THRESHOLD 0.001
#define MIN_ROUGHNESS 0.001

// Some drivers require the following
//precision highp float;

// Geometry buffers
layout(location = 0) out vec3 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec3 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;
layout(location = 4) out vec4 matPropertiesBuffer;

// Variables from vertex shader
in mat3 TBN;
in mat3 normalMatrix;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;
in vec3 tangentFragPos;
in vec3 tangentCameraPos;
in float parallaxScale;
in float parallaxLOD;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;

/* Current combined texture channels:
	Red: Roughness
	Green: Metalness
	Blue: Height
	Alpha: Ambient Occlusion */
uniform sampler2D combinedTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D noiseTexture;

uniform float alphaThreshold;
uniform float emissiveMultiplier;
uniform float emissiveThreshold;
uniform float heightScale;
uniform float stochasticSamplingScale;
uniform ivec2 screenSize;
uniform vec2 pomNumOfSteps;	// Parallax Occlusion Mapping number of depth steps (x = min, y = max)

/*
vec2 steepParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    return currentTexCoords;
}*/


float sum(vec3 p_value) 
{
	return p_value.x + p_value.y + p_value.z; 
}

float sum(vec4 p_value) 
{
	return p_value.x + p_value.y + p_value.z + p_value.w; 
}

vec4 textureNoTile(in sampler2D p_texture, vec2 p_texCoords, float p_scale)
{
    float k = texture(noiseTexture, 0.005 * p_texCoords).r; // cheap (cache friendly) lookup
    
    vec2 duvdx = dFdx(p_texCoords);
    vec2 duvdy = dFdy(p_texCoords);
    
    float l = k * 8.0;
    float f = fract(l);
    
#if STOCHASTIC_SAMPLING_MIPMAP_SEAM_FIX
	// Avoid mipmap seams
    float ia = floor(l + 0.5);
    float ib = floor(l);
    f = min(f, 1.0 - f) * 2.0;
#else
    float ia = floor(l);
    float ib = ia + 1.0;
#endif    
    
    vec2 offa = sin(vec2(3.0, 7.0) * ia); // can replace with any other hash
    vec2 offb = sin(vec2(3.0, 7.0) * ib); // can replace with any other hash

    vec4 cola = textureGrad(p_texture, p_texCoords + p_scale * offa, duvdx, duvdy).xyzw;
    vec4 colb = textureGrad(p_texture, p_texCoords + p_scale * offb, duvdx, duvdy).xyzw;
    
    return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum(cola - colb)));
}

// Returns texture data with or without texture repetition algorithm 
vec4 sampleTexture(const sampler2D p_texture, const vec2 p_texCoords)
{
#if STOCHASTIC_SAMPLING
	return textureNoTile(p_texture, p_texCoords, stochasticSamplingScale);
#else
	return texture(p_texture, p_texCoords).rgba;
#endif
}

float getHeight(const vec2 p_texCoords)
{
	return 1.0 - sampleTexture(combinedTexture, p_texCoords).b;
}

float getHeight(const vec2 p_texCoords, const vec2 p_dPdx, const vec2 p_dPdy)
{
	return textureGrad(combinedTexture, p_texCoords, p_dPdx, p_dPdy).b;
}

float getRoughness(const vec2 p_texCoords)
{
	return max(sampleTexture(combinedTexture, p_texCoords).r, MIN_ROUGHNESS);
}

float getMetalness(const vec2 p_texCoords)
{
	return sampleTexture(combinedTexture, p_texCoords).g;
}

float getAmbientOcclusion(const vec2 p_texCoords)
{
	return sampleTexture(combinedTexture, p_texCoords).a;
}

vec3 getNormalFromMap(vec2 p_texCoord, vec3 p_worldPos)
{
    vec3 tangentNormal = texture(normalTexture, p_texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(p_worldPos);
    vec3 Q2  = dFdy(p_worldPos);
    vec2 st1 = dFdx(p_texCoord);
    vec2 st2 = dFdy(p_texCoord);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 barycentricToModelPosition(vec2 p_texCoords, vec3 p_vertex0, vec3 p_vertex1, vec3 p_vertex2) 
{
    vec3 barycentric = vec3(1.0 - p_texCoords.x - p_texCoords.y, p_texCoords.x, p_texCoords.y);
    return barycentric.x * p_vertex0 + barycentric.y * p_vertex1 + barycentric.z * p_vertex2;
}

vec2 calcSimpleParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{
	float height =  getHeight(p_texCoords) * heightScale;    
    vec2 p = p_viewDir.xy * height;
    return p_texCoords - p;
}

vec2 calcReliefParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{   
	// determine required number of layers
	float numLayers = mix(pomNumOfSteps.y, pomNumOfSteps.x, abs(dot(vec3(0, 0, 1), p_viewDir)));

	// height of each layer
	float layerHeight = 1.0 / numLayers;
	// depth of current layer
	float currentLayerHeight = 0;
	// shift of texture coordinates for each iteration
	vec2 dtex = 0.01 * p_viewDir.xy / p_viewDir.z / numLayers;

	// current texture coordinates
	vec2 currentTextureCoords = p_texCoords;

	// depth from heightmap
	float heightFromTexture = getHeight(currentTextureCoords);

	// while point is above surface
	while(heightFromTexture > currentLayerHeight)
	{
	  // go to the next layer
	  currentLayerHeight += layerHeight;
	  // shift texture coordinates along V
	  currentTextureCoords -= dtex;
	  // new depth from heightmap
	  heightFromTexture = getHeight(currentTextureCoords);
	}

	///////////////////////////////////////////////////////////
	// Start of Relief Parallax Mapping

	// decrease shift and height of layer by half
	vec2 deltaTexCoord = dtex / 2;
	float deltaHeight = layerHeight / 2;

	// return to the mid point of previous layer
	currentTextureCoords += deltaTexCoord;
	currentLayerHeight -= deltaHeight;

	// binary search to increase precision of Steep Paralax Mapping
	const int numSearches = 5;
	for(int i = 0; i < numSearches; i++)
	{
	  // decrease shift and height of layer by half
	  deltaTexCoord /= 2;
	  deltaHeight /= 2;

	  // new depth from heightmap
	  heightFromTexture = getHeight(currentTextureCoords);

	  // shift along or against vector V
	  if(heightFromTexture > currentLayerHeight) // below the surface
	  {
		 currentTextureCoords -= deltaTexCoord;
		 currentLayerHeight += deltaHeight;
	  }
	  else // above the surface
	  {
		 currentTextureCoords += deltaTexCoord;
		 currentLayerHeight -= deltaHeight;
	  }
	}

	return currentTextureCoords;
}

vec2 calcSteepParallaxMapping(const vec2 p_texCoords, const vec3 p_viewDir)
{ 
    // Calculate the number of depth layers
    const float numLayers = mix(pomNumOfSteps.y, pomNumOfSteps.x, abs(dot(vec3(0.0, 0.0, 1.0), p_viewDir)));
	
    // Calculate the size of each layer
    const float layerDepth = 1.0 / numLayers;
	
    // Depth of the current layer
    float currentLayerDepth = 0.0;
	
    // Calculate the amount to shift the texture coordinates per layer
    const vec2 rayDirection = p_viewDir.xy / p_viewDir.z * heightScale; 
    const vec2 deltaTexCoords = rayDirection / numLayers;
  
    // Get initial values
    vec2 currentTexCoords = p_texCoords;
    float currentDepthMapValue = getHeight(currentTexCoords);
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // Shift texture coordinates along the direction of ray
        currentTexCoords -= deltaTexCoords;
		
        // Get the depth value at current texture coordinates
        currentDepthMapValue = getHeight(currentTexCoords);  
		
        // Get the depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    return currentTexCoords;
}

vec2 calcParallaxMappingAlternative(vec2 T, vec3 V)
{
	vec2 finalTexCoords = T;

	// Calculate the parallax offset vector max length.
	// This is equivalent to the tangent of the angle between the
	// viewer position and the fragment location.
	float fParallaxLimit = -length(V.xy) / V.z;

	// Scale the parallax limit according to heightmap scale.
	fParallaxLimit *= heightScale;						

	// Calculate the parallax offset vector direction and maximum offset.
	vec2 vOffsetDir = normalize(V.xy);
	vec2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Calculate the geometric surface normal vector, the vector from
	// the viewer to the fragment, and the vector from the fragment
	// to the light.
	vec3 N = normalize(normal);
	vec3 E = normalize(V);

	// Calculate how many samples should be taken along the view ray
	// to find the surface intersection.  This is based on the angle
	// between the surface normal and the view vector.
	float nNumSamples2 = mix(pomNumOfSteps.y, pomNumOfSteps.x, dot(E, N));
	int nNumSamples = int(nNumSamples2);
	
	// Specify the view ray step size.  Each sample will shift the current
	// view ray by this amount.
	float fStepSize = 1.0 / nNumSamples;

	// Calculate the texture coordinate partial derivatives in screen
	// space for the tex2Dgrad texture sampling instruction.
	vec2 dx = dFdx(T);
	vec2 dy = dFdy(T);

	// Initialize the starting view ray height and the texture offsets.
	float fCurrRayHeight = 1.0;	
	vec2 vCurrOffset = vec2(0, 0);
	vec2 vLastOffset = vec2(0, 0);

	float fLastSampledHeight = 1;
	float fCurrSampledHeight = 1;

	int nCurrSample = 0;

	while(nCurrSample < nNumSamples)
	{
		// Sample the heightmap at the current texcoord offset.  The heightmap 
		// is stored in the alpha channel of the height/normal map.
		//fCurrSampledHeight = tex2Dgrad( NH_Sampler, IN.texcoord + vCurrOffset, dx, dy ).a;
		fCurrSampledHeight = getHeight(T + vCurrOffset, dx, dy); //NormalHeightMap.SampleGrad( LinearSampler, IN.texcoord + vCurrOffset, dx, dy ).a;

		// Test if the view ray has intersected the surface.
		if(fCurrSampledHeight > fCurrRayHeight)
		{
			// Find the relative height delta before and after the intersection.
			// This provides a measure of how close the intersection is to 
			// the final sample location.
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = ( fCurrRayHeight + fStepSize ) - fLastSampledHeight;
			float ratio = delta1/(delta1+delta2);

			// Interpolate between the final two segments to 
			// find the true intersection point offset.
			vCurrOffset = ratio * vLastOffset + (1.0 - ratio) * vCurrOffset;
			
			// Force the exit of the while loop
			nCurrSample = nNumSamples + 1;	
		}
		else
		{
			// The intersection was not found.  Now set up the loop for the next
			// iteration by incrementing the sample count,
			nCurrSample++;

			// take the next view ray height step,
			fCurrRayHeight -= fStepSize;
			
			// save the current texture coordinate offset and increment
			// to the next sample location, 
			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			// and finally save the current heightmap height.
			fLastSampledHeight = fCurrSampledHeight;
		}
	}

	// Calculate the final texture coordinate at the intersection point.
	vec2 vFinalCoords = T + vCurrOffset;

	return vFinalCoords;
}

vec2 calcParallaxOcclusionMapping(const vec2 p_texCoords, const vec3 p_viewDir)
{
    // Calculate the number of depth layers
    const float numLayers = mix(pomNumOfSteps.y, pomNumOfSteps.x, abs(dot(vec3(0.0, 0.0, 1.0), p_viewDir)));  
	
    // Calculate the size of each layer
    const float layerDepth = 1.0 / numLayers;
	
    // Depth of the current layer
    float currentLayerDepth = 0.0;
	
    // Calculate the amount to shift the texture coordinates per layer
    const vec2 rayDirection = p_viewDir.xy / p_viewDir.z * heightScale; 
    const vec2 deltaTexCoords = rayDirection / numLayers;
  
    // Get initial values
    vec2 currentTexCoords = p_texCoords;
    float currentDepthMapValue = getHeight(currentTexCoords);
	
    while(currentLayerDepth < currentDepthMapValue)
    {
        // Shift texture coordinates along the direction of ray
        currentTexCoords -= deltaTexCoords;
		
        // Get the depth value at current texture coordinates
        currentDepthMapValue = getHeight(currentTexCoords);
		
        // Get the depth of next layer
        currentLayerDepth += layerDepth;  
    }
    	
    // Get the texture coordinates before collision (reverse operations)
    const vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // Get the depth after and before collision for linear interpolation
    const float afterDepth  = currentDepthMapValue - currentLayerDepth;
    const float beforeDepth = getHeight(prevTexCoords) - currentLayerDepth + layerDepth;
 
    // Calculate the interpolation of texture coordinates
    const float weight = afterDepth / (afterDepth - beforeDepth);
    return prevTexCoords * weight + currentTexCoords * (1.0 - weight);
}

void main(void)
{
	float height = getHeight(texCoord);
	
	vec2 newCoords = texCoord;
		
#if PARALLAX_MAPPING
	vec3 viewDirNew = normalize(tangentCameraPos - tangentFragPos);
	
#if PARALLAX_MAPPING_METHOD == 1
    newCoords = calcSimpleParallaxMapping(newCoords, viewDirNew);
#elif PARALLAX_MAPPING_METHOD == 2
    newCoords = calcReliefParallaxMapping(newCoords, viewDirNew);
#elif PARALLAX_MAPPING_METHOD == 3
    newCoords = calcSteepParallaxMapping(newCoords, viewDirNew);
#elif PARALLAX_MAPPING_METHOD == 4
    newCoords = calcParallaxMappingAlternative(newCoords, viewDirNew);
#elif PARALLAX_MAPPING_METHOD == 5
    newCoords = calcParallaxOcclusionMapping(newCoords, viewDirNew);
#endif

#endif
	
	// Get diffuse color
	vec4 diffuseColor = sampleTexture(diffuseTexture, newCoords);
	
	// Discard fragment if the diffuse alpha color value is smaller than alpha threshold
	if(diffuseColor.a < alphaThreshold)
		discard;
	
	// Get material properties values with the new coordinates
	// R - roughness
	// G - metalness
	// B - height
	// A - ambient occlusion
	vec4 combinedTextureColor = sampleTexture(combinedTexture, newCoords);
	
	// Get the emissive color with the new coordinates
	vec4 emissiveColor = sampleTexture(emissiveTexture, newCoords);
	
	// Get the normal map values
	vec3 normalColor = sampleTexture(normalTexture, newCoords).rgb;
	
	// Use emissive alpha channel as an intensity multiplier
	emissiveColor *= emissiveMultiplier;// * emissiveColor.a;
	
	// Write roughness, metalness to the material properties buffer
	matPropertiesBuffer = vec4(
		max(combinedTextureColor.x, MIN_ROUGHNESS), 	// R - roughness
		combinedTextureColor.y, 						// G - metalness
		combinedTextureColor.w, 						// B - ambient occlusion
		1.0);											// A - unused for now
		
	// Write diffuse color to the diffuse buffer
	diffuseBuffer = diffuseColor;
	
	// Write emissive color into the emissive buffer
	emissiveBuffer = emissiveColor;
	
	// Write fragment's position in world space	to the position buffer
	positionBuffer = fragPos;
		
	// Write fragment's normal direction in world space
	//normalBuffer = normalize(TBN * normalize(texture(normalTexture, newCoords).rgb * 2.0 - 1.0));
	normalBuffer = normalize(TBN * normalize(normalColor * 2.0 - 1.0));
}
