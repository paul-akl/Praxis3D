#version 430 core

#define GHOST_TINT_PER_SAMPLE          1  // Apply txGhostGradientColor inside the sample loop instead of at the end.
#define DISABLE_HALO_ASPECT_RATIO      0  // Code is simpler/cheaper without this, but the halo shape is fixed.
#define DISABLE_CHROMATIC_ABERRATION   0  // Takes 3x fewer samples.

layout(location = 0) out vec4 diffuseBuffer;
//layout(location = 1) out vec4 colorBuffer;

flat in float aspectRatio;

uniform ivec2 screenSize;
//uniform sampler2D emissiveMap;
uniform sampler2D inputColorMap;
uniform sampler2D ghostGradientTexture;

struct LensFlareParameters
{
	int m_ghostCount;
	float m_ghostSpacing;
	float m_ghostThreshold;

	float m_haloAspectRatio;
	float m_haloRadius;
	float m_haloThickness;
	float m_haloThreshold;
	
	float m_chromaticAberration;
	float m_lensFlaireDownsample;
};

layout (std140) uniform LensFlareParametersBuffer
{
	LensFlareParameters lensFlareParam;
};

// Cubic window; map [0, p_radius] in [1, 0] as a cubic falloff from p_center.
float cubicFalloff(float p_x, float p_center, float p_radius)
{
	p_x = min(abs(p_x - p_center) / p_radius, 1.0);
	return 1.0 - p_x * p_x * (3.0 - 2.0 * p_x);
}

vec3 applyThreshold(in vec3 p_color, in float p_threshold)
{
	return max(p_color - vec3(p_threshold), vec3(0.0));
}

vec3 sampleSceneColor(in vec2 p_texCoord)
{
#if DISABLE_CHROMATIC_ABERRATION
	return textureLod(inputColorMap, p_texCoord, lensFlareParam.m_lensFlaireDownsample).rgb;
#else
	vec2 offset = normalize(vec2(0.5) - p_texCoord) * lensFlareParam.m_chromaticAberration;
	return vec3(
		textureLod(inputColorMap, p_texCoord + offset, lensFlareParam.m_lensFlaireDownsample).r,
		textureLod(inputColorMap, p_texCoord, lensFlareParam.m_lensFlaireDownsample).g,
		textureLod(inputColorMap, p_texCoord - offset, lensFlareParam.m_lensFlaireDownsample).b
		);
#endif
}

vec3 sampleGhosts(in vec2 p_texCoord, in float p_threshold)
{
	vec3 ghostColor = vec3(0.0);
	vec2 ghostVec = (vec2(0.5) - p_texCoord) * lensFlareParam.m_ghostSpacing;
	for (int i = 0; i < lensFlareParam.m_ghostCount; ++i)
	{
		// sample scene color
		vec2 suv = fract(p_texCoord + ghostVec * vec2(i));
		vec3 s = sampleSceneColor(suv);
		s = applyThreshold(s, p_threshold);
		
		// tint / weight
		float distanceToCenter = distance(suv, vec2(0.5));
		#if GHOST_TINT_PER_SAMPLE
			// incorporate weight into tint gradient
			s *= textureLod(ghostGradientTexture, vec2(distanceToCenter, 0.5), 0.0).rgb; 
		#else
			// analytical weight
			float weight = 1.0 - smoothstep(0.0, 0.75, distanceToCenter); // analytical weight
			s *= weight;
		#endif

		ghostColor += s;
	}
	#if !GHOST_TINT_PER_SAMPLE
		ghostColor *= textureLod(ghostGradientTexture, vec2(distance(p_texCoord, vec2(0.5)), 0.5), 0.0).rgb;
	#endif

	return ghostColor;
}

vec3 sampleHalo(in vec2 p_texCoord, in float p_radius, in float p_aspectRatio, in float p_threshold)
{
	vec2 haloVec = vec2(0.5) - p_texCoord;
	
	#if DISABLE_HALO_ASPECT_RATIO
		haloVec = normalize(haloVec);
		float haloWeight = distance(p_texCoord, vec2(0.5));
	#else
		haloVec.x /= p_aspectRatio;
		haloVec = normalize(haloVec);
		haloVec.x *= p_aspectRatio;
		vec2 wuv = (p_texCoord - vec2(0.5, 0.0)) / vec2(p_aspectRatio, 1.0) + vec2(0.5, 0.0);
		float haloWeight = distance(wuv, vec2(0.5));
	#endif
	
	haloVec *= p_radius;
	haloWeight = cubicFalloff(haloWeight, p_radius, lensFlareParam.m_haloThickness);
	
	return applyThreshold(sampleSceneColor(p_texCoord + haloVec), p_threshold) * haloWeight;
}
vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	vec2 texCoordFlipped = vec2(1.0) - texCoord;
	
	vec3 lensFlareColor = vec3(0.0);
	lensFlareColor += sampleGhosts(texCoordFlipped, lensFlareParam.m_ghostThreshold);
	lensFlareColor += sampleHalo(
		texCoordFlipped, 
		lensFlareParam.m_haloRadius, 
		aspectRatio, 
		lensFlareParam.m_haloThreshold);
	
	// Write the colors to the framebuffers
	diffuseBuffer = vec4(lensFlareColor, 1.0);
}