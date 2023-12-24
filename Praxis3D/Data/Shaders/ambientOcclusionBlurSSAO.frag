#version 430 core

out vec4 outputColor;

uniform sampler2D finalColorMap;

uniform ivec2 screenSize;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main() 
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
	// Calculate offset scale for the blur sampling
    vec2 texelSize = 1.0 / vec2(textureSize(finalColorMap, 0));
	
    float result = 0.0;
	
	// Go over each vertical sample pixel
    for(int x = -2; x < 2; ++x) 
    {
		// Go over each horizontal sample pixel
        for(int y = -2; y < 2; ++y) 
        {
			// Offset the sample position
            vec2 offset = vec2(float(x), float(y)) * texelSize;
			
			// Accumulate the result
            result += texture(finalColorMap, texCoord + offset).z;
        }
    }
	
	// Average the result
    float occlusion = result / (4.0 * 4.0);
	
	// Get the material properties value and set the occlusion value
	vec4 matProperties = texture(finalColorMap, texCoord);
	matProperties.z = occlusion;
	
    outputColor = matProperties;
}  