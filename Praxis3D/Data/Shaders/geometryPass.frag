#version 330 core

// Some drivers require the following
//precision highp float;

// Geometry buffers
layout(location = 0) out vec4 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec4 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;

// Variables from vertex shader
in vec3 fragPos;
in vec2 texCoord;
in vec3 normal;
in mat3 TBN;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D glossTexture;
uniform sampler2D heightTexture;

uniform float alphaThreshold;
uniform float emissiveThreshold;
uniform float parallaxHeightScale;
uniform vec3 cameraPosVec;

vec2 simpleParallaxMapping(vec2 p_texCoords, vec3 p_viewDir, float p_height)
{
	vec2 parallax = p_viewDir.xy / p_viewDir.z * (p_height * 0.1f);
    return p_texCoords + parallax; 
}

vec2 steepParallaxMapping(in vec2 T, in vec3 V)//, out float parallaxHeight)
{
   // determine number of layers from angle between V and N
   const float minLayers = 5;
   const float maxLayers = 15;
   float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), V)));

   // height of each layer
   float layerHeight = 1.0 / numLayers;
   // depth of current layer
   float currentLayerHeight = 0;
   // shift of texture coordinates for each iteration
   vec2 dtex = 0.1 * V.xy / V.z / numLayers;

   // current texture coordinates
   vec2 currentTextureCoords = T;

   // get first depth from heightmap
   float heightFromTexture = texture(heightTexture, currentTextureCoords).r;

   // while point is above surface
   while(heightFromTexture > currentLayerHeight)
   {
      // to the next layer
      currentLayerHeight += layerHeight;
      // shift texture coordinates along vector V
      currentTextureCoords -= dtex;
      // get new depth from heightmap
      heightFromTexture = texture(heightTexture, currentTextureCoords).r;
   }

   // return results
   //parallaxHeight = currentLayerHeight;
   return currentTextureCoords;
} 

vec2 parallaxMapping(in vec2 T, in vec3 V)//, out float parallaxHeight)
{
   // determine optimal number of layers
   const float minLayers = 15;
   const float maxLayers = 25;
   float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), V)));

   // height of each layer
   float layerHeight = 1.0 / numLayers;
   // current depth of the layer
   float curLayerHeight = 0.1;
   // shift of texture coordinates for each layer
   vec2 dtex = 0.015 * V.xy / V.z / numLayers;

   // current texture coordinates
   vec2 currentTextureCoords = T;

   // depth from heightmap
   float heightFromTexture = texture(heightTexture, currentTextureCoords).r;

   // while point is above the surface
   while(heightFromTexture > curLayerHeight)
   {
      // to the next layer
      curLayerHeight += layerHeight;
      // shift of texture coordinates
      currentTextureCoords -= dtex;
      // new depth from heightmap
      heightFromTexture = texture(heightTexture, currentTextureCoords).r;
   }

   ///////////////////////////////////////////////////////////

   // previous texture coordinates
   vec2 prevTCoords = currentTextureCoords + dtex;

   // heights for linear interpolation
   float nextH = heightFromTexture - curLayerHeight;
   float prevH = texture(heightTexture, prevTCoords).r
                           - curLayerHeight + layerHeight;
						   
   // proportions for linear interpolation
   float weight = nextH / (nextH - prevH);

   // interpolation of texture coordinates
   vec2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0 - weight);

   // interpolation of depth values
   //parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);

   // return result
   return finalTexCoords;
}

vec2 reliefParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{    
	//float v = height * 0.04 - 0.02;
	//vec2 newCoords = texCoord + (viewDir.xy * v);
	
	// number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), p_viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = p_viewDir.xy / p_viewDir.z * 0.01;
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = p_texCoords;
    float currentDepthMapValue = texture(heightTexture, currentTexCoords).r;
		  
    while(currentLayerDepth < currentDepthMapValue)
    {		
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightTexture, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
	//return currentTexCoords;
	
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightTexture, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

vec2 reliefParallaxMapping2(vec2 p_texCoords, vec3 p_viewDir)
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
	float heightFromTexture = texture(heightTexture, currentTextureCoords).r;

	// while point is above surface
	while(heightFromTexture > currentLayerHeight)
	{
	  // go to the next layer
	  currentLayerHeight += layerHeight;
	  // shift texture coordinates along V
	  currentTextureCoords -= dtex;
	  // new depth from heightmap
	  heightFromTexture = texture(heightTexture, currentTextureCoords).r;
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
	  heightFromTexture = texture(heightTexture, currentTextureCoords).r;

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

	// return results
	//parallaxHeight = currentLayerHeight;    
	return currentTextureCoords;
}

vec2 parallaxOcclusionMapping(vec2 p_texCoords, vec3 p_viewDir)
{
	// determine optimal number of layers
   const float minLayers = 10;
   const float maxLayers = 15;
   float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), p_viewDir)));

   // height of each layer
   float layerHeight = 1.0 / numLayers;
   // current depth of the layer
   float curLayerHeight = 0.0;
   // shift of texture coordinates for each layer
   vec2 dtex = 0.012 * p_viewDir.xy / p_viewDir.z / numLayers;

   // current texture coordinates
   vec2 currentTextureCoords = p_texCoords;

   // depth from heightmap
   float heightFromTexture = texture(heightTexture, currentTextureCoords).r;

   // while point is above the surface
   while(heightFromTexture > curLayerHeight)
   {
      // to the next layer
      curLayerHeight += layerHeight;
      // shift of texture coordinates
      currentTextureCoords -= dtex;
      // new depth from heightmap
      heightFromTexture = texture(heightTexture, currentTextureCoords).r;
   }

   ///////////////////////////////////////////////////////////

   // previous texture coordinates
   vec2 prevTCoords = currentTextureCoords + dtex;

   // heights for linear interpolation
   float nextH = heightFromTexture - curLayerHeight;
   float prevH = texture(heightTexture, prevTCoords).r
                           - curLayerHeight + layerHeight;

   // proportions for linear interpolation
   float weight = nextH / (nextH - prevH);

   // interpolation of texture coordinates
   vec2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0 - weight);

   // interpolation of depth values
   //parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);

   // return result
   return finalTexCoords;
}

vec2 parallaxOcclusionMapping2(vec2 p_texCoords, vec3 p_viewDir)
{
	float distanceToFrag = length(fragPos - cameraPosVec);
	
	if(distanceToFrag > 29.0)
		return p_texCoords;
	
    // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 15;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), p_viewDir)));
	
	if(distanceToFrag > 15.0)
		numLayers = min(((30.0 - distanceToFrag) / 30.0), 1.0) * numLayers;
		
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = p_viewDir.xy / p_viewDir.z * 0.02;
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = p_texCoords;
    float currentDepthMapValue = texture(heightTexture, currentTexCoords).r;
	float previousDepth = currentDepthMapValue;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightTexture, currentTexCoords).r;  
		
		previousDepth = currentDepthMapValue;
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightTexture, prevTexCoords).r - currentLayerDepth + layerDepth;
    //float beforeDepth = previousDepth - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

/*vec2 distanceFragment(vec2 p_texCoords, vec3 p_viewDir)
{
//(v2fConnector v2f,

  //uniform sampler2D colorTex,

  //uniform sampler2D normalTex,

  //uniform sampler3D distanceTex,

  //uniform float3 normalizationFactor)

//{
  //f2fConnector f2f;

  // Normalize the offset vector in texture space.
  // The normalization factor ensures we are normalized with respect
  // to a distance which is defined in terms of pixels.
  float3 offset = normalize(p_viewDir);
  //offset *= normalizationFactor;

  float3 texCoord = p_texCoords;

  // March a ray
  for(int i = 0; i < 10; i++) 
  {
    float dist = texture(heightTexture, texCoord).r;
    texCoord += dist * offset;
  }

  // Compute derivatives of unperturbed texcoords.
  // This is because the offset texcoords will have discontinuities
  // which lead to incorrect filtering.
  vec2 dx = dFdx(p_texCoords);
  vec2 dy = dFdy(p_texCoords);

  // Do bump-mapped lighting in tangent space.
  // 'normalTex' stores tangent-space normals remapped
  // into the range [0, 1].
  vec3 tanNormal = 2 * f3tex2D(normalTex, texCoord.xy, dx, dy) - 1;
  vec3 tanLightVec = normalize(v2f.tanLightVec);
  float diffuse = dot(tanNormal, tanLightVec);

  // Multiply diffuse lighting by texture color
  f2f.COL.rgb = diffuse * f3tex2D(colorTex, texCoord.xy, dx, dy);
  f2f.COL.a = 1;

  return f2f;
}*/

vec2 steepParallax(vec2 p_texCoords, vec3 p_viewDir)
{
	// We are at height bumpScale.  March forward until we hit a hair or the 
    // base surface.  Instead of dropping down discrete y-voxels we should be
    // marching in texels and dropping our y-value accordingly (TODO: fix)
    float height = 1.0;

    // Number of height divisions
    float numSteps = 5;

    /** Texture coordinate marched forward to intersection point */
    vec2 offsetCoord = p_texCoords.xy;
    vec4 NB = texture2D(heightTexture, offsetCoord);

    vec3 tsE = normalize(p_viewDir);

    // Increase steps at oblique angles
    // Note: tsE.z = N dot V
    numSteps = mix(numSteps*2, numSteps, p_viewDir.z);

    // We have to negate tsE because we're walking away from the eye.
    //vec2 delta = vec2(-_tsE.x, _tsE.y) * bumpScale / (_tsE.z * numSteps);
    float step;
    vec2 delta;


    // Constant in z
    step = 1.0 / numSteps;
    delta = vec2(-p_viewDir.x, p_viewDir.y) * 1.0 / (p_viewDir.z * numSteps);

        // Can also step along constant in xy; the results are essentially
        // the same in each case.
        // delta = 1.0 / (25.6 * numSteps) * vec2(-tsE.x, tsE.y);
        // step = tsE.z * bumpScale * (25.6 * numSteps) / (length(tsE.xy) * 400);

    while (NB.r < height) 
	{
        height -= step;
        offsetCoord += delta;
        NB = texture2D(heightTexture, offsetCoord);
    }

    height = NB.r;
	
	return offsetCoord;
	
    // Choose the color at the location we hit
    //const vec3 color = texture2D(texture, offsetCoord).rgb;
/*
    tsL = normalize(tsL);

    // Use the normals out of the bump map
    vec3 tsN = NB.xyz * 2 - 1;

    // Smooth normals locally along gradient to avoid making slices visible.
    // The magnitude of the step should be a function of the resolution and
    // of the bump scale and number of steps.
//    tsN = normalize(texture2D(normalBumpMap, offsetCoord + vec2(tsN.x, -tsN.y) * mipScale).xyz * 2 - 1 + tsN);

    const vec3 tsH = normalize(tsL + normalize(_tsE));

    const float NdotL = max(0, dot(tsN, tsL));
    const float NdotH = max(0, dot(tsN, tsH));
    float spec = NdotH * NdotH;

    vec3 lightColor = vec3(1.5, 1.5, 1) * 0.9;
    vec3 ambient = vec3(0.4,0.4,0.6) * 1.4;

    float selfShadow = 0;

    // Don't bother checking for self-shadowing if we're on the
    // back side of an object.
    if (NdotL > 0) {

        // Trace a shadow ray along the light vector.
        const int numShadowSteps = mix(60,5,tsL.z);
        step = 1.0 / numShadowSteps;
        delta = vec2(tsL.x, -tsL.y) * bumpScale / (numShadowSteps * tsL.z);

            // We start one iteration out to avoid shadow acne 
            // (could start bumped a little without going
            // a whole iteration).
            height = NB.a + step * 0.1;

            while ((NB.a < height) && (height < 1)) {
                height += step;
                offsetCoord += delta;
                NB = texture2D(normalBumpMap, offsetCoord);
            }

            // We are in shadow if we left the loop because
            // we hit a point
            selfShadow = (NB.a < height);

            // Shadows will make the whole scene darker, so up the light contribution
            lightColor = lightColor * 1.2;
        }
        }

        gl_FragColor.rgb = 
            color * ambient + color * NdotL * selfShadow * lightColor;*/
}

vec2 steepParallax2(vec2 p_texCoords, vec3 p_viewDir)
{
	/*   // diffuse 
    uniform sampler2D tex0;
     //normal map and height map 
    uniform sampler2D tex1;
     
    varying vec3 Normal;
    varying vec3 Tangent;
    varying vec3 Binormal;
     
    varying vec3 LightDirectionTS;
    varying vec3 EyeDirectionTS;
     
    varying float Depth;
     
    vec3 GetNormal( vec2 coord )
    {
    vec3 ret = texture2D( tex1, coord ).xyz;
    ret.xy = ret.xy * 2.0 - 1.0;
     
    ret.y *= -1.0;
    return normalize( ret );
    }
     
    float GetHeight( vec2 coord )
    {
    return texture2D( tex1, coord ).w;
    }
     
    void main()
    {*/
	
	float Depth = 1.0;
	//vec3 LightDirection = normalize( LightDirectionTS );
	vec3 EyeDirection = normalize( p_viewDir );
	vec3 EyeRay = p_viewDir;
	vec2 startCoord = p_texCoords;
	vec2 newCoord = startCoord;
 
	// Common for Parallax
	vec2 ParallaxXY = ( EyeRay ).xy/-EyeRay.z * Depth;
	
	// Steep Parallax
	float Step = 0.01;
	vec2 dt = ParallaxXY * Step;
	float Height =0.5;
	float oldHeight = 0.5;
	vec2 Coord = startCoord;
	vec2 oldCoord = Coord;
	float HeightMap = texture2D(heightTexture, Coord).r;
	float oldHeightMap = HeightMap;
 
	while( HeightMap < Height )
	{
		oldHeightMap = HeightMap;
		oldHeight = Height;
		oldCoord = Coord;
 
		Height -= Step;
		Coord += dt;
		HeightMap = texture2D(heightTexture, Coord).r;
	}
	//if( Coord.s <= 0.0 || Coord.t <= 0.0 || Coord.s >= 1.0 || Coord.t >= 1.0 )
	// discard;
	Coord = (Coord + oldCoord) * 0.5;
	if( Height < 0.0 )
	{
		Coord = oldCoord;
		Height = 0.0;
	}
	// else // interpolation
	// {
	// float ds = (GetHeight( oldCoord ) - oldHeight) * StepXYlength / ( -Step - oldHeightMap + HeightMap );
	// Coord = oldCoord + dt * ds;
	// Height = oldHeight + -Step/StepXYlength * ds;
	// }
	newCoord = Coord;
	return Coord;
 
	/*vec4 colorMap = vec4( texture2D( tex0, newCoord ).xyz, 1.0 );
	float heightMap = GetHeight( newCoord );
	vec3 normalMap = GetNormal( newCoord );
 
	// Ambient
	vec4 ambient_color = (gl_FrontLightModelProduct.sceneColor) +
	(colorMap * gl_LightSource[0].ambient * gl_FrontMaterial.ambient);
 
	// Diffuse
	float lambertTerm = max( dot( normalMap, LightDirection ), 0.0 );
	vec4 diffuse_color = colorMap * gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * lambertTerm;
 
	// Specular
	vec3 reflectDirection = 2.0 * dot( normalMap, LightDirection ) * normalMap - LightDirection; // in Tangen Space
 
	float specular = pow( max( dot(reflectDirection, EyeDirectionTS), 0.1 ), gl_FrontMaterial.shininess );
	vec4 specular_color = gl_LightSource[0].specular * gl_FrontMaterial.specular * specular;
 
	// if( newCoord.s < 0.0 || newCoord.t < 0.0 || newCoord.s > 1.0 || newCoord.t > 1.0 )
	// {
	// discard;
	// }
 
	gl_FragColor = vec4( ambient_color.xyz + diffuse_color.xyz + specular_color.xyz, 1.0 );*/
}

vec2 simpleParallaxMapping(vec2 p_texCoords, vec3 p_viewDir)
{ 
    //float height =  texture(heightTexture, p_texCoords).r;     
    //return p_texCoords - (p_viewDir.xy ) * (height * 0.01);
	
	float height =  texture(heightTexture, p_texCoords).r;    
    vec2 p = p_viewDir.xy  * (height * 0.02);
    return p_texCoords - p;
	
}

void main(void)
{	
	// Simple parallax mapping (offset texture coordinates based on height)
	//float height = texture(heightTexture, texCoord).r;
	//float v = height * 0.04 - 0.02;
	//vec2 newCoords = texCoord + (viewDir.xy * v);
	
    //vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	
    //vec3 viewDir2 = normalize((TBN * cameraPosVec) - (TBN * fragPos));
   
	//vec3 viewDir2 = normalize(TBN * (cameraPosVec - fragPos).xyz);
	//viewDir2.x = -viewDir2.x;
	//vec3 viewDir2 = normalize(cameraPosVec - fragPos);
	//vec3 viewDir2 = normalize(TangentViewPos - TangentFragPos);
	//vec3 viewDir2 = normalize(viewDir);
	
	//vec2 newCoords = parallaxMapping(viewDir, texCoord);
	//vec2 newCoords = reliefParallaxMapping2(texCoord, viewDir);
	//vec2 newCoords = texCoord;
	
	//vec2 newCoords = simpleParallaxMapping(texCoord, viewDir, texture(heightTexture, texCoord).r);
	//vec3 et = viewDir;
	
	//float h = 0.04 * (1.0 - texture(heightTexture, texCoord).r) + 0.02;

                                                       // now offset texture coordinates
                                                       // with height
    //vec2 tex = texCoord - et.xy * h;// / et.z;
	
	//float height = texture(heightTexture, texCoord.st).r;
	//float v = height * 0.04 - 0.02;
	//tex = texCoord + (viewDir.xy * v);
	
	/*const float numSteps  = 10.0;

	//vec3 et = vec3 ( dot ( e, t ), dot ( e, b ), dot ( e, n ) );
    float step   = 1.0 / numSteps;
    vec2  dtex   = et.xy * 0.1 / ( numSteps * et.z );  // adjustment for one layer
    float height = 1.0;                                  // height of the layer
    vec2  tex    = texCoord.xy;                   // our initial guess
    float h      = texture2D ( heightTexture, tex ).r;       // get height

    if ( h < height )
    {
        height -= step;
        tex    += dtex;
        h       = texture2D ( heightTexture, tex ).r;

        if ( h < height )
        {
            height -= step;
            tex    += dtex;
            h       = texture2D ( heightTexture, tex ).r;

            if ( h < height )
            {
                height -= step;
                tex    += dtex;
                h       = texture2D ( heightTexture, tex ).r;

                if ( h < height )
                {
                    height -= step;
                    tex    += dtex;
                    h       = texture2D ( heightTexture, tex ).r;

                    if ( h < height )
                    {
                        height -= step;
                        tex    += dtex;
                        h       = texture2D ( heightTexture, tex ).r;
                    }
                }
            }
        }
    }*/
	
	//vec2 newCoords = tex;
	
	/*float bumpScale = -10.0;
     
    // normalize the other tangent space vectors
    vec3 viewVector = normalize(TangentViewPos - TangentFragPos);
     
    vec3 tsE = normalize(TangentViewPos);
 
    const float numSteps = 30.0; // How many steps the UV ray tracing should take
    float height = 1.0;
    float step = 1.0 / numSteps;
 
    vec2 offset = texCoord;
    vec4 NB = texture2D(heightTexture, offset);
 
    vec2 delta = vec2(tsE.x, tsE.y) * bumpScale / (tsE.z * numSteps);
 
    // find UV offset
    for (float i = 0.0; i < numSteps; i++) 
	{
        if (NB.a < height) 
		{
            height -= step;
            offset += delta;
            NB = texture2D(heightTexture, offset);
        } 
		else 
		{
            break;
        }
    }
	
	vec2 newCoords = offset;*/
	
    /*vec3 color = texture2D(base, offset).rgb;
     
     
        vec3 normal = texture2D(map, offset).rgb * 2.0 - 1.0;
 
    // calculate this pixel's lighting
        float nxDir = max(0.0, dot(normal, lightVector));
        vec3 ambient = ambientColor * color;
 
        float specularPower = 0.0;
        if(nxDir != 0.0)
        {
                vec3 halfVector = normalize(lightVector + viewVector);
                float nxHalf = max(0.0, dot(normal, halfVector));
                specularPower = pow(nxHalf, shininess);
        }
        vec3 specular = specularColor * specularPower;
     
     
        vec3 pixel_color = ambient + (diffuseColor * nxDir * color) + specular;
     
    // find shadowing if enabled
        if (shadow == 1.0) {
            vec2 shadow_offset = offset;
        vec3 tsH = normalize(lightVector + tsE);
        float NdotL = max(0.0, dot(normal, lightVector));
         
        float selfShadow = 0.0;
         
        if (NdotL > 0.0) {
             
            const float numShadowSteps = 10.0;
            step = 1.0 / numShadowSteps;
            delta = vec2(lightVector.x, lightVector.y) * bumpScale / (numShadowSteps * lightVector.z);
             
            height = NB.a + step * .1;
             
            for (float i = 0.0; i < numShadowSteps; i++) {
                if (NB.a < height && height < 1.0) {
                    height += step;
                    shadow_offset += delta;
                    NB = texture2D(map, shadow_offset);
                } else {
                    break;
                }
            }
             
            selfShadow = float(NB.a < height);
             
        }
         
        if (selfShadow == 0.0) {
        pixel_color *= .5;
        }
    }
     
    gl_FragColor = vec4(pixel_color, 1.0);
     
    if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0) {
        gl_FragColor.a = 0.0;
    }*/
	
	vec3 viewDir = normalize(TBN * (cameraPosVec - fragPos).xyz * vec3(-1.0, 1.0, 1.0));
	vec2 newCoords = texCoord;
	
	//if(distanceToFrag < 30.0)
	newCoords = parallaxOcclusionMapping2(texCoord, viewDir);
	//vec2 newCoords = texCoord;
	
	// Get diffuse color
	vec4 diffuse = texture(diffuseTexture, newCoords).rgba;
	
	// Discard fragment if the diffuse alpha color value is smaller than alpha threshold
	if(diffuse.a < alphaThreshold)
		discard;
	
	// Get gloss and specular values and emissive color
	//float gloss = texture(glossTexture, newCoords).r;
	float gloss = texture(specularTexture, newCoords).g;
	float specular = texture(specularTexture, newCoords).r;
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
	// Write emissive color into the emissive buffer
	emissiveBuffer = emissiveColor;
	// Write fragment's position in world space	to the position buffer, write gloss value in alpha channel
	positionBuffer = vec4(fragPos, gloss);
	// Perform normal mapping and write the new normal to the normal buffer, write specular value in alpha channel
	normalBuffer = vec4(TBN * normalize(texture(normalTexture, newCoords).rgb * 2.0 - 1.0), specular);
	//normalBuffer = vec4(normalize(normal), specular);
}
