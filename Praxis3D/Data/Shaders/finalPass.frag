/*
	Final pass shader, fragment (finalPass.frag)
	Performs FXAA anti-aliasing. Also used to copy color data to the default framebuffer to show on screen.
*/
#version 430 core

// Settings for FXAA.
#define FXAA 1
#define FXAA_EDGE_THRESHOLD_MIN 0.0312
#define FXAA_EDGE_THRESHOLD_MAX 0.125
#define FXAA_ITERATIONS 12
#define FXAA_SUBPIXEL_QUALITY 0.75
#define FXAA_QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))

out vec4 outputColor;

uniform ivec2 screenSize;
uniform vec2 inverseScreenSize;

uniform sampler2D inputColorMap;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

// Convert RGB colors to a luma value
float rgb2luma(const vec3 p_color)
{
    return sqrt(dot(p_color, vec3(0.299, 0.587, 0.114)));
}

// Performs Fast Approximate Anti-Aliasing on the given texture
vec3 fxaa(const sampler2D p_colorMap, const vec2 p_texCoord)
{
	vec3 colorCenter = texture(p_colorMap, p_texCoord).rgb;
	
	// Luma at the current fragment
	const float lumaCenter = rgb2luma(colorCenter);
	
	// Luma at the four direct neighbours of the current fragment.
	const float lumaDown = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(0, -1)).rgb);
	const float lumaUp = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(0, 1)).rgb);
	const float lumaLeft = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(-1, 0)).rgb);
	const float lumaRight = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(1, 0)).rgb);
	
	// Find the maximum and minimum luma around the current fragment.
	const float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	const float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	// Compute the delta.
	const float lumaRange = lumaMax - lumaMin;
	
	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
	if(lumaRange < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD_MAX))
		return colorCenter;
	
	// Query the 4 remaining corners lumas.
	const float lumaDownLeft = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(-1, -1)).rgb);
	const float lumaUpRight = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(1, 1)).rgb);
	const float lumaUpLeft = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(-1, 1)).rgb);
	const float lumaDownRight = rgb2luma(textureOffset(p_colorMap, p_texCoord, ivec2(1, -1)).rgb);

	// Combine the four edges lumas (using intermediary variables for future computations with the same values).
	const float lumaDownUp = lumaDown + lumaUp;
	const float lumaLeftRight = lumaLeft + lumaRight;

	// Same for corners
	const float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	const float lumaDownCorners = lumaDownLeft + lumaDownRight;
	const float lumaRightCorners = lumaDownRight + lumaUpRight;
	const float lumaUpCorners = lumaUpRight + lumaUpLeft;

	// Compute an estimation of the gradient along the horizontal and vertical axis.
	const float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
	const float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

	// Is the local edge horizontal or vertical ?
	const bool isHorizontal = (edgeHorizontal >= edgeVertical);
	
	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	const float luma1 = isHorizontal ? lumaDown : lumaLeft;
	const float luma2 = isHorizontal ? lumaUp : lumaRight;
	
	// Compute gradients in this direction.
	const float gradient1 = luma1 - lumaCenter;
	const float gradient2 = luma2 - lumaCenter;

	// Which direction is the steepest ?
	const bool is1Steepest = abs(gradient1) >= abs(gradient2);

	// Gradient in the corresponding direction, normalized.
	const float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
	
	// Choose the step size (one pixel) according to the edge direction.
	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	// Average luma in the correct direction.
	float lumaLocalAverage = 0.0;

	if(is1Steepest)
	{
		// Switch the direction
		stepLength = - stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	} 
	else 
	{
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}

	// Shift UV in the correct direction by half a pixel.
	vec2 currentUv = p_texCoord;
	if(isHorizontal)
	{
		currentUv.y += stepLength * 0.5;
	} 
	else 
	{
		currentUv.x += stepLength * 0.5;
	}
	
	// Compute offset (for each iteration step) in the right direction.
	const vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);
	
	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
	vec2 uv1 = currentUv - offset;
	vec2 uv2 = currentUv + offset;

	// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
	float lumaEnd1 = rgb2luma(texture(p_colorMap, uv1).rgb);
	float lumaEnd2 = rgb2luma(texture(p_colorMap, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	// If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	// If the side is not reached, we continue to explore in this direction.
	if(!reached1)
	{
		uv1 -= offset;
	}
	if(!reached2)
	{
		uv2 += offset;
	}
	
	// If both sides have not been reached, continue to explore.
	if(!reachedBoth)
	{
		for(int i = 2; i < FXAA_ITERATIONS; i++){
			// If needed, read luma in 1st direction, compute delta.
			if(!reached1)
			{
				lumaEnd1 = rgb2luma(texture(p_colorMap, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			// If needed, read luma in opposite direction, compute delta.
			if(!reached2)
			{
				lumaEnd2 = rgb2luma(texture(p_colorMap, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable quality.
			if(!reached1)
			{
				uv1 -= offset * FXAA_QUALITY(i);
			}
			if(!reached2)
			{
				uv2 += offset * FXAA_QUALITY(i);
			}

			// If both sides have been reached, stop the exploration.
			if(reachedBoth)
			{
				break;
			}
		}
	}
	
	// Compute the distances to each extremity of the edge.
	const float distance1 = isHorizontal ? (p_texCoord.x - uv1.x) : (p_texCoord.y - uv1.y);
	const float distance2 = isHorizontal ? (uv2.x - p_texCoord.x) : (uv2.y - p_texCoord.y);

	// In which direction is the extremity of the edge closer ?
	const bool isDirection1 = distance1 < distance2;
	const float distanceFinal = min(distance1, distance2);

	// Length of the edge.
	const float edgeThickness = (distance1 + distance2);

	// UV offset: read in the direction of the closest side of the edge.
	const float pixelOffset = -distanceFinal / edgeThickness + 0.5;
	
	// Is the luma at center smaller than the local average ?
	const bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	// (in the direction of the closer side of the edge.)
	const bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

	// If the luma variation is incorrect, do not offset.
	float finalOffset = correctVariation ? pixelOffset : 0.0;
	
	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	const float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	
	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	const float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	const float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	
	// Compute a sub-pixel offset based on this delta.
	const float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * FXAA_SUBPIXEL_QUALITY;

	// Pick the biggest of the two offsets.
	finalOffset = max(finalOffset, subPixelOffsetFinal);
	
	// Compute the final UV coordinates.
	vec2 finalUv = p_texCoord;
	if(isHorizontal)
	{
		finalUv.y += finalOffset * stepLength;
	} 
	else 
	{
		finalUv.x += finalOffset * stepLength;
	}

	// Read the color at the new UV coordinates, and use it.
	return texture(p_colorMap, finalUv).rgb;
}

void main(void)
{	
	// Calculate screen-space texture coordinates, for buffer access
	const vec2 texCoord = calcTexCoord();
	
#if FXAA
	// Perform Fast Approximate Anti-Aliasing
	const vec3 fragmentColor = fxaa(inputColorMap, texCoord);
#else
	// Get the color data from the final color map
	const vec3 fragmentColor = texture(inputColorMap, texCoord).xyz;
#endif
	
	// Write the color to the framebuffer
	outputColor = vec4(fragmentColor, 1.0);
}