#version 330 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out vec3 worldPos;
out vec2 texCoord;
out vec3 normal;
out vec3 eyeDir;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;

void main(void)
{
	vec4 viewPosition = projMat * modelViewMat * vec4(vertexPosition,1.0);
	mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
	
    worldPos = (modelMat * vec4(vertexPosition, 1.0)).xyz;
	texCoord = textureCoord;
	normal = (modelMat * vec4(vertexNormal, 0.0)).xyz;
	eyeDir = normalize(-viewPosition).xyz;
	TBN = mat3(normalMatrix * vertexTangent, normalMatrix * vertexBitangent, normalMatrix * vertexNormal);

	//gl_Position = viewPosition;
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}