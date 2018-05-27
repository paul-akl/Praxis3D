#version 430 core
//precision mediump float;

out vec4 outputColor;

uniform sampler2D inputColorMap;
uniform ivec2 screenSize;

uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Calculate the size of a single pixel
	vec2 texCoordOffset = 1.0 / screenSize;
	
	// Get current fragment color
	vec3 fragColor = texture(inputColorMap, texCoord).xyz * weight[0];
	
	// Sample color around adjacent fragments
	for(int i = 1; i < 3; i++) 
	{
		fragColor += texture(inputColorMap, texCoord + vec2(0.0, offset[i] / screenSize.y)).xyz * weight[i];
		fragColor += texture(inputColorMap, texCoord - vec2(0.0, offset[i] / screenSize.y)).xyz * weight[i];
	}
	
	outputColor = vec4(fragColor, 1.0);
}