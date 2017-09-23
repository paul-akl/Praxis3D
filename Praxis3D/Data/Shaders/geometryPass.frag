#version 430 core

#define HEIGHT_SCALE_THRESHOLD 0.001
#define ROUGHNESS_MIN 0.001

// Some drivers require the following
//precision highp float;

// Geometry buffers
layout(location = 0) out vec3 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec3 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;
layout(location = 4) out vec4 matPropertiesBuffer;


uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;

// Variables from vertex shader
in mat3 TBN;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;
in vec3 tangentFragPos;
in vec3 tangentCameraPos;
in float parallaxScale;

/* Current combined texture channels:
	Red: Roughness
	Green: Metalness
	Blue: Height
	Alpha: Ambient Occlusion
*/
uniform sampler2D combinedTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;

uniform float alphaThreshold;
uniform float emissiveThreshold;

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

float getHeight(const vec2 p_texCoords)
{
	return 1.0 - texture(combinedTexture, p_texCoords).b;
}
float getHeight(const vec2 p_texCoords, const vec2 p_dPdx, const vec2 p_dPdy)
{
	return textureGrad(combinedTexture, p_texCoords, p_dPdx , p_dPdy).b;
}

float getRoughness(const vec2 p_texCoords)
{
	return max(texture(combinedTexture, p_texCoords).r, ROUGHNESS_MIN);
}
float getMetalness(const vec2 p_texCoords)
{
	return texture(combinedTexture, p_texCoords).g;
}
float getAmbientOcclusion(const vec2 p_texCoords)
{
	return texture(combinedTexture, p_texCoords).a;
}

vec2 reliefParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{   
	// determine required number of layers
	const float minLayers = 10;
	const float maxLayers = 15;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), p_viewDir)));

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

	  // shift along or agains vector V
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

vec2 parallaxMappingNew(vec2 T, vec3 V)
{
	vec2 finalTexCoords = T;

	// Calculate the parallax offset vector max length.
	// This is equivalent to the tangent of the angle between the
	// viewer position and the fragment location.
	float fParallaxLimit = -length( V.xy ) / V.z;

	// Scale the parallax limit according to heightmap scale.
	fParallaxLimit *= 0.04f;						

	// Calculate the parallax offset vector direction and maximum offset.
	vec2 vOffsetDir = normalize( V.xy );
	vec2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Calculate the geometric surface normal vector, the vector from
	// the viewer to the fragment, and the vector from the fragment
	// to the light.
	vec3 N = normalize( normal );
	vec3 E = normalize( V );

	// Calculate how many samples should be taken along the view ray
	// to find the surface intersection.  This is based on the angle
	// between the surface normal and the view vector.
	float nNumSamples2 = mix( 20, 10, dot( E, N ) );
	int nNumSamples = int(nNumSamples2);
	
	// Specify the view ray step size.  Each sample will shift the current
	// view ray by this amount.
	float fStepSize = 1.0 / nNumSamples;

	// Calculate the texture coordinate partial derivatives in screen
	// space for the tex2Dgrad texture sampling instruction.
	vec2 dx = dFdx( T );
	vec2 dy = dFdy( T );

	// Initialize the starting view ray height and the texture offsets.
	float fCurrRayHeight = 1.0;	
	vec2 vCurrOffset = vec2( 0, 0 );
	vec2 vLastOffset = vec2( 0, 0 );

	float fLastSampledHeight = 1;
	float fCurrSampledHeight = 1;

	int nCurrSample = 0;

	while ( nCurrSample < nNumSamples )
	{
		// Sample the heightmap at the current texcoord offset.  The heightmap 
		// is stored in the alpha channel of the height/normal map.
		//fCurrSampledHeight = tex2Dgrad( NH_Sampler, IN.texcoord + vCurrOffset, dx, dy ).a;
		fCurrSampledHeight = getHeight(T + vCurrOffset, dx, dy); //NormalHeightMap.SampleGrad( LinearSampler, IN.texcoord + vCurrOffset, dx, dy ).a;

		// Test if the view ray has intersected the surface.
		if ( fCurrSampledHeight > fCurrRayHeight )
		{
			// Find the relative height delta before and after the intersection.
			// This provides a measure of how close the intersection is to 
			// the final sample location.
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = ( fCurrRayHeight + fStepSize ) - fLastSampledHeight;
			float ratio = delta1/(delta1+delta2);

			// Interpolate between the final two segments to 
			// find the true intersection point offset.
			vCurrOffset = (ratio) * vLastOffset + (1.0-ratio) * vCurrOffset;
			
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

	// Use the final texture coordinates to get the normal vector, then 
	// expand it from [0,1] to [-1,1] range.
	//vec4 vFinalNormal = NormalHeightMap.Sample( LinearSampler, vFinalCoords ); //.a;
		
	// Sample the colormap at the final intersection point.
	//vec4 vFinalColor = ColorMap.Sample( LinearSampler, vFinalCoords );

	// Expand the final normal vector from [0,1] to [-1,1] range.
	//vFinalNormal = vFinalNormal * 2.0f - 1.0f;

	// Shade the fragment based on light direction and normal.
	//vec3 vAmbient = vFinalColor.rgb * 0.1f;
	//vec3 vDiffuse = vFinalColor.rgb * max( 0.0f, dot( L, vFinalNormal.xyz ) ) * 0.5f;
	//vFinalColor.rgb = vAmbient + vDiffuse;

	//OUT.color = vFinalColor;

	return vFinalCoords;
}

vec2 parallaxOcclusionMapping(vec2 texCoords, vec3 viewDir, float p_LOD)
{
	// number of depth layers
    const float minLayers = 15;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir))) * p_LOD;  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * parallaxScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = getHeight(currentTexCoords);
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = getHeight(currentTexCoords);  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = getHeight(prevTexCoords) - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec2 simpleParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{ 
    //float height =  texture(heightTexture, p_texCoords).r;     
    //return p_texCoords - (p_viewDir.xy ) * (height * 0.01);
	
	float height =  getHeight(p_texCoords);    
    vec2 p = p_viewDir.xy  * (height * 0.02);
    return p_texCoords - p;
}

void main(void)
{ 	
	vec3 viewDir = tangentCameraPos - tangentFragPos;
	float distanceToFrag = length(viewDir);
	viewDir = normalize(viewDir);
	
	vec2 newCoords = texCoord;
	
	//if(distanceToFrag > 29.0)
	//	return p_texCoords;
	
    // number of depth layers
	
	//if(distanceToFrag > 5.0)
	//	numLayers = min(((20.0 - distanceToFrag) / 20.0), 1.0) * numLayers;
	newCoords = parallaxOcclusionMapping(texCoord, viewDir, 1.0);
	if(distanceToFrag < 9.90)
	{
		float LOD = min(((10.0 - distanceToFrag) / 10.0), 1.0);
		float LOD2 = clamp((distanceToFrag - 8.0) / 2.0, 0.0, 1.0);
	
		//if(parallaxScale > HEIGHT_SCALE_THRESHOLD)
		//	newCoords = mix(parallaxOcclusionMapping(texCoord, viewDir, LOD), texCoord, 1.0 - LOD);
	}
	//if(distanceToFrag < 30.0)
	//	newCoords = parallaxMappingNew2(texCoord, viewDir);
	//vec2 newCoords = texCoord;
	
	// discards a fragment when sampling outside default texture region (fixes border artifacts)
    //if(newCoords.x > 1.0 || newCoords.y > 1.0 || newCoords.x < 0.0 || newCoords.y < 0.0)
    //    discard;
	//vec2 testColor = parallaxMappingNew(texCoord, eye);
	
	// Get diffuse color
	vec4 diffuse = texture(diffuseTexture, newCoords).rgba;
	
	// Discard fragment if the diffuse alpha color value is smaller than alpha threshold
	if(diffuse.a < alphaThreshold)
		discard;
	
	// Get roughness and metalness values, and emissive color
	float roughness = getRoughness(newCoords);
	float metalness = getMetalness(newCoords);
	vec4 emissiveColor = texture(emissiveTexture, newCoords).rgba;
	
	// Apply emissive color only if it's above the threshold
	if(emissiveColor.a > emissiveThreshold)
	{
		// Use emissive alpha channel as an intensity multiplier
		emissiveColor *= emissiveColor.a;
	}
	else
	{
		emissiveColor = vec4(0.0);
	}
	
	// Write diffuse color to the diffuse buffer
	diffuseBuffer = diffuse;
	// Write roughness, metalness to the material properties buffer
	matPropertiesBuffer = vec4(roughness, metalness, 1.0, 1.0);
	// Write emissive color into the emissive buffer
	emissiveBuffer = emissiveColor;
	// Write fragment's position in world space	to the position buffer
	positionBuffer = fragPos;
	// Perform normal mapping and write the new normal to the normal buffer
	normalBuffer = TBN * normalize(texture(normalTexture, newCoords).rgb * 2.0 - 1.0);
}