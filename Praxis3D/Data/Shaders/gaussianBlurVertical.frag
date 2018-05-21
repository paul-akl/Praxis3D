#version 430 core
//precision mediump float;

in vec2 blurTexCoords[14];

//out vec4 blurBuffer;
out vec4 outputColor;
//layout(location = 0) out vec4 blurBuffer;

uniform sampler2D inputColorMap;
uniform ivec2 screenSize;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
    vec3 fragColor = vec3(0.0);
	
	float blurOffset = 1.0;
	
	// Calculate the size of a single pixel
	vec2 texCoordOffset = 1.0 / screenSize;
	
    fragColor = texture(inputColorMap, texCoord).rgb * weight[0]; // current fragment's contribution
	
	for(int i = 1; i < 5; ++i)
	{
		fragColor += texture(inputColorMap, texCoord + vec2(0.0, texCoordOffset.y * i)).rgb * weight[i];
		fragColor += texture(inputColorMap, texCoord - vec2(0.0, texCoordOffset.y * i)).rgb * weight[i];
	}
		
	/*
    blurTexCoords[ 0] = texCoord + vec2(0.0, -0.028 * blurOffset);
    blurTexCoords[ 1] = texCoord + vec2(0.0, -0.024 * blurOffset);
    blurTexCoords[ 2] = texCoord + vec2(0.0, -0.020 * blurOffset);
    blurTexCoords[ 3] = texCoord + vec2(0.0, -0.016 * blurOffset);
    blurTexCoords[ 4] = texCoord + vec2(0.0, -0.012 * blurOffset);
    blurTexCoords[ 5] = texCoord + vec2(0.0, -0.008 * blurOffset);
    blurTexCoords[ 6] = texCoord + vec2(0.0, -0.004 * blurOffset);
    blurTexCoords[ 7] = texCoord + vec2(0.0,  0.004 * blurOffset);
    blurTexCoords[ 8] = texCoord + vec2(0.0,  0.008 * blurOffset);
    blurTexCoords[ 9] = texCoord + vec2(0.0,  0.012 * blurOffset);
    blurTexCoords[10] = texCoord + vec2(0.0,  0.016 * blurOffset);
    blurTexCoords[11] = texCoord + vec2(0.0,  0.020 * blurOffset);
    blurTexCoords[12] = texCoord + vec2(0.0,  0.024 * blurOffset);
    blurTexCoords[13] = texCoord + vec2(0.0,  0.028 * blurOffset);
	*/
	/*
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.028 * blurOffset)).xyz * 0.0044299121055113265;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.024 * blurOffset)).xyz * 0.00895781211794;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.020 * blurOffset)).xyz * 0.0215963866053;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.016 * blurOffset)).xyz * 0.0443683338718;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.012 * blurOffset)).xyz * 0.0776744219933;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.008 * blurOffset)).xyz * 0.115876621105;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0, -0.004 * blurOffset)).xyz * 0.147308056121;
    fragColor += texture(emissiveMap, texCoord         			  ).xyz * 0.259576912161;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.004 * blurOffset)).xyz * 0.147308056121;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.008 * blurOffset)).xyz * 0.115876621105;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.012 * blurOffset)).xyz * 0.0776744219933;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.016 * blurOffset)).xyz * 0.0443683338718;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.020 * blurOffset)).xyz * 0.0215963866053;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.024 * blurOffset)).xyz * 0.00895781211794;
    fragColor += texture(emissiveMap, texCoord + vec2(0.0,  0.028 * blurOffset)).xyz * 0.0044299121055113265;
	*/
	/*
    fragColor += texture(emissiveMap, blurTexCoords[ 0]).xyz * 0.0044299121055113265;
    fragColor += texture(emissiveMap, blurTexCoords[ 1]).xyz * 0.00895781211794;
    fragColor += texture(emissiveMap, blurTexCoords[ 2]).xyz * 0.0215963866053;
    fragColor += texture(emissiveMap, blurTexCoords[ 3]).xyz * 0.0443683338718;
    fragColor += texture(emissiveMap, blurTexCoords[ 4]).xyz * 0.0776744219933;
    fragColor += texture(emissiveMap, blurTexCoords[ 5]).xyz * 0.115876621105;
    fragColor += texture(emissiveMap, blurTexCoords[ 6]).xyz * 0.147308056121;
    fragColor += texture(emissiveMap, texCoord         ).xyz * 0.259576912161;
    fragColor += texture(emissiveMap, blurTexCoords[ 7]).xyz * 0.147308056121;
    fragColor += texture(emissiveMap, blurTexCoords[ 8]).xyz * 0.115876621105;
    fragColor += texture(emissiveMap, blurTexCoords[ 9]).xyz * 0.0776744219933;
    fragColor += texture(emissiveMap, blurTexCoords[10]).xyz * 0.0443683338718;
    fragColor += texture(emissiveMap, blurTexCoords[11]).xyz * 0.0215963866053;
    fragColor += texture(emissiveMap, blurTexCoords[12]).xyz * 0.00895781211794;
    fragColor += texture(emissiveMap, blurTexCoords[13]).xyz * 0.0044299121055113265;
	*/
	outputColor = vec4(fragColor, 1.0);
	//outputColor = vec4(fragColor, 1.0);
	//outputColor = vec4(1.0, 0.0, 0.0, 1.0);
    //blurBuffer = vec4(texture(emissiveMap, texCoord).xyz, 1.0);
}