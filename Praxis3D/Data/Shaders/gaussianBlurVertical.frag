#version 430 core
//precision mediump float;

in vec2 blurTexCoords[14];

out vec4 outputColor;

uniform sampler2D emissiveTexture;
uniform ivec2 screenSize;

vec2 calcTexCoord(void)
{
    return gl_FragCoord.xy / screenSize;
}

void main(void)
{
	// Calculate screen-space texture coordinates, for buffer access
	vec2 texCoord = calcTexCoord();
	
    vec3 fragColor = vec3(0.0);
	
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
	
	outputColor = vec4(fragColor, 1.0);
}