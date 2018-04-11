#version 430 core

out vec2 blurTexCoords[14];

uniform float blurOffset;

void main(void)
{
    blurTexCoords[ 0] = texCoord + vec2(0.0, -0.028*blurOffset);
    blurTexCoords[ 1] = texCoord + vec2(0.0, -0.024*blurOffset);
    blurTexCoords[ 2] = texCoord + vec2(0.0, -0.020*blurOffset);
    blurTexCoords[ 3] = texCoord + vec2(0.0, -0.016*blurOffset);
    blurTexCoords[ 4] = texCoord + vec2(0.0, -0.012*blurOffset);
    blurTexCoords[ 5] = texCoord + vec2(0.0, -0.008*blurOffset);
    blurTexCoords[ 6] = texCoord + vec2(0.0, -0.004*blurOffset);
    blurTexCoords[ 7] = texCoord + vec2(0.0,  0.004*blurOffset);
    blurTexCoords[ 8] = texCoord + vec2(0.0,  0.008*blurOffset);
    blurTexCoords[ 9] = texCoord + vec2(0.0,  0.012*blurOffset);
    blurTexCoords[10] = texCoord + vec2(0.0,  0.016*blurOffset);
    blurTexCoords[11] = texCoord + vec2(0.0,  0.020*blurOffset);
    blurTexCoords[12] = texCoord + vec2(0.0,  0.024*blurOffset);
    blurTexCoords[13] = texCoord + vec2(0.0,  0.028*blurOffset);
	
	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}
