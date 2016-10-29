#version 410 core

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

out vec3 tangent;
out vec3 bitangent;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;

void main(void)
{
	//temp_tangent = ( 0, 0, 1 )
	//bitangent = cross( temp_tangent, normal )
	//tangent = cross( normal, bitangent )
	//float3x3 TBN = float3x3( tangent, bitangent, normal )

	
	//vec4 viewPosition = projMat * modelViewMat * vec4(vertexPosition,1.0);
	mat3 normalMatrix = transpose(inverse(mat3(modelViewMat)));
	
    worldPos = (modelMat * vec4(vertexPosition, 1.0)).xyz;
	texCoord = textureCoord;
	normal = (modelMat * vec4(vertexNormal, 0.0)).xyz;
	//eyeDir = normalize(-viewPosition).xyz;
	
	vec3 tangent = vec3(0.0, 0.0, 1.0);
	vec3 bitangent = cross(tangent, vertexNormal);
	tangent = cross(normal, bitangent);
	
	TBN = mat3(normalMatrix * tangent, normalMatrix * bitangent, normalMatrix * vertexNormal);
	//TBN = normalMatrix;

	tangent = vertexTangent;
	bitangent = vertexBitangent;
	
	//gl_Position = viewPosition;
	//gl_Position = vec4(vertexPosition, 1.0);
}