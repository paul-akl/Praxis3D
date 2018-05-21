#version 430 core

uniform mat4 transposeViewMat;
uniform mat4 atmScatProjMat;

out vec3 viewRay;

void main(void) 
{	
	// Determine texture coordinates
	vec2 texCoord = vec2((gl_VertexID == 2) ?  2.0 :  0.0, (gl_VertexID == 1) ?  2.0 :  0.0);
	
	// Calculate the position, so that the triangle fills the whole screen
	vec4 vertexPos = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
		
	viewRay = (((transposeViewMat)) * vec4((atmScatProjMat * vertexPos).xyz, 0.0)).xyz;
	gl_Position = vertexPos;
}