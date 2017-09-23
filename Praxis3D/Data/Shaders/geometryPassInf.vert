#version 430 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out vec3 fragPos;
out vec3 normal;
out vec3 tangentFragPos;
out vec3 tangentCameraPos;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 cameraPosVec;

void main(void)
{		
	// Multiply position and normal by model matrix (to convert them into world space)
    fragPos = vec3(modelMat * vec4(vertexPosition, 1.0));
	normal = normalize(mat3(modelMat) * vertexNormal);
				
	// Compute TBN matrix
    vec3 T = normalize(mat3(modelMat) * vertexTangent);
    vec3 B = normalize(mat3(modelMat) * vertexBitangent);
	
	TBN = transpose(inverse(mat3(T, B, normal)));
	mat3 TBN2 = transpose((mat3(T, B, normal)));
	
	tangentCameraPos = TBN2 * cameraPosVec;
	tangentFragPos = TBN2 * fragPos;
		
	gl_Position = projMat * viewMat * vec4(fragPos, 1.0);
}