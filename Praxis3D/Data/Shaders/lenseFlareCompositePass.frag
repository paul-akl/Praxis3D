#version 430 core

layout(location = 0) out vec4 colorBuffer;

flat in float starburstOffset;

uniform ivec2 screenSize;

uniform sampler2D diffuseMap;
uniform sampler2D inputColorMap;
uniform sampler2D lensDirtTexture;
uniform sampler2D lenseStarburstTexture;

float saturate(float p_value)
{
	return clamp(p_value, 0.0f, 1.0f);
}

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Get the current fragment color
	vec3 fragmentColor = texture(inputColorMap, texCoord).xyz;
	
	// Calculate starburst
	vec2 centerVec = texCoord - vec2(0.5);
	float d = length(centerVec);
	float radial = acos(centerVec.x / d);
	float mask = 
		  texture(lenseStarburstTexture, vec2(radial + starburstOffset * 1.0, 0.0)).r * 
		  texture(lenseStarburstTexture, vec2(radial - starburstOffset * 0.5, 0.0)).r;
		  
	mask = saturate(mask + (1.0 - smoothstep(0.0, 0.3, d)));
	
	// Apply lens dirt
	mask *= textureLod(lensDirtTexture, texCoord, 0.0).r * 0.5;
	
	vec3 lensFlareColor = textureLod(diffuseMap, texCoord, 0.0).rgb * mask;
	
	// Write the colors to the framebuffers
	colorBuffer = vec4(fragmentColor + lensFlareColor, 1.0);
}