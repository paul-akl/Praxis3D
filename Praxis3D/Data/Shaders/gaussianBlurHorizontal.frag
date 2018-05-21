#version 430 core
//precision mediump float;

in vec2 blurTexCoords[14];

out vec4 outputColor;

uniform sampler2D inputColorMap;
uniform ivec2 screenSize;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
 
//uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
//uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

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
		fragColor += texture(inputColorMap, texCoord + vec2(texCoordOffset.x * i, 0.0)).rgb * weight[i];
		fragColor += texture(inputColorMap, texCoord - vec2(texCoordOffset.x * i, 0.0)).rgb * weight[i];
	}
	
	//FragmentColor = texture2D( image, vec2(gl_FragCoord)/1024.0 ) * weight[0];
   /* for(int i=1; i<3; i++) 
	{
        fragColor +=
            texture2D( inputColorMap, (texCoord + vec2(0.0, offset[i])) / 900.0) * weight[i];
        fragColor +=
            texture2D( inputColorMap, (texCoord - vec2(0.0, offset[i])) / 900.0) * weight[i];
    }*/
	/*
    blurTexCoords[ 0] = texCoord + vec2(-0.028 * blurOffset, 0.0);
    blurTexCoords[ 1] = texCoord + vec2(-0.024 * blurOffset, 0.0);
    blurTexCoords[ 2] = texCoord + vec2(-0.020 * blurOffset, 0.0);
    blurTexCoords[ 3] = texCoord + vec2(-0.016 * blurOffset, 0.0);
    blurTexCoords[ 4] = texCoord + vec2(-0.012 * blurOffset, 0.0);
    blurTexCoords[ 5] = texCoord + vec2(-0.008 * blurOffset, 0.0);
    blurTexCoords[ 6] = texCoord + vec2(-0.004 * blurOffset, 0.0);
    blurTexCoords[ 7] = texCoord + vec2( 0.004 * blurOffset, 0.0);
    blurTexCoords[ 8] = texCoord + vec2( 0.008 * blurOffset, 0.0);
    blurTexCoords[ 9] = texCoord + vec2( 0.012 * blurOffset, 0.0);
    blurTexCoords[10] = texCoord + vec2( 0.016 * blurOffset, 0.0);
    blurTexCoords[11] = texCoord + vec2( 0.020 * blurOffset, 0.0);
    blurTexCoords[12] = texCoord + vec2( 0.024 * blurOffset, 0.0);
    blurTexCoords[13] = texCoord + vec2( 0.028 * blurOffset, 0.0);
	*/
	/*
    fragColor += texture(blurMap, texCoord + vec2(-0.028 * blurOffset, 0.0)).xyz * 0.0044299121055113265;
    fragColor += texture(blurMap, texCoord + vec2(-0.024 * blurOffset, 0.0)).xyz * 0.00895781211794;
    fragColor += texture(blurMap, texCoord + vec2(-0.020 * blurOffset, 0.0)).xyz * 0.0215963866053;
    fragColor += texture(blurMap, texCoord + vec2(-0.016 * blurOffset, 0.0)).xyz * 0.0443683338718;
    fragColor += texture(blurMap, texCoord + vec2(-0.012 * blurOffset, 0.0)).xyz * 0.0776744219933;
    fragColor += texture(blurMap, texCoord + vec2(-0.008 * blurOffset, 0.0)).xyz * 0.115876621105;
    fragColor += texture(blurMap, texCoord + vec2(-0.004 * blurOffset, 0.0)).xyz * 0.147308056121;
    fragColor += texture(blurMap, texCoord         			  ).xyz * 0.259576912161;
    fragColor += texture(blurMap, texCoord + vec2(0.004 * blurOffset, 0.0)).xyz * 0.147308056121;
    fragColor += texture(blurMap, texCoord + vec2(0.008 * blurOffset, 0.0)).xyz * 0.115876621105;
    fragColor += texture(blurMap, texCoord + vec2(0.012 * blurOffset, 0.0)).xyz * 0.0776744219933;
    fragColor += texture(blurMap, texCoord + vec2(0.016 * blurOffset, 0.0)).xyz * 0.0443683338718;
    fragColor += texture(blurMap, texCoord + vec2(0.020 * blurOffset, 0.0)).xyz * 0.0215963866053;
    fragColor += texture(blurMap, texCoord + vec2(0.024 * blurOffset, 0.0)).xyz * 0.00895781211794;
    fragColor += texture(blurMap, texCoord + vec2(0.028 * blurOffset, 0.0)).xyz * 0.0044299121055113265;
	*/
	/*
    fragColor += texture(blurMap, blurTexCoords[ 0]).xyz * 0.0044299121055113265;
    fragColor += texture(blurMap, blurTexCoords[ 1]).xyz * 0.00895781211794;
    fragColor += texture(blurMap, blurTexCoords[ 2]).xyz * 0.0215963866053;
    fragColor += texture(blurMap, blurTexCoords[ 3]).xyz * 0.0443683338718;
    fragColor += texture(blurMap, blurTexCoords[ 4]).xyz * 0.0776744219933;
    fragColor += texture(blurMap, blurTexCoords[ 5]).xyz * 0.115876621105;
    fragColor += texture(blurMap, blurTexCoords[ 6]).xyz * 0.147308056121;
    fragColor += texture(blurMap, texCoord         ).xyz * 0.259576912161;
    fragColor += texture(blurMap, blurTexCoords[ 7]).xyz * 0.147308056121;
    fragColor += texture(blurMap, blurTexCoords[ 8]).xyz * 0.115876621105;
    fragColor += texture(blurMap, blurTexCoords[ 9]).xyz * 0.0776744219933;
    fragColor += texture(blurMap, blurTexCoords[10]).xyz * 0.0443683338718;
    fragColor += texture(blurMap, blurTexCoords[11]).xyz * 0.0215963866053;
    fragColor += texture(blurMap, blurTexCoords[12]).xyz * 0.00895781211794;
    fragColor += texture(blurMap, blurTexCoords[13]).xyz * 0.0044299121055113265;
	*/
	outputColor = vec4(fragColor, 1.0);
	//outputColor = vec4(0.0, 1.0, 1.0, 1.0);
	//outputColor = vec4(texture(blurMap, texCoord).xyz, 1.0);
}