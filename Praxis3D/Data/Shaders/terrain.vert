#version 400 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out vec3 worldPos;
//out vec2 texCoord;
out vec3 normal;
out vec3 eyeDir;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;

uniform sampler2D diffuseTexture;
uniform sampler2D heightTexture;

void main(void)
{
	vec2 texCoord = vertexPosition.xy;
	//texCoord = textureCoord;
	float height = texture(diffuseTexture, vertexPosition.xy).a;
	vec4 displaced = vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0);
	//vec4 displaced = vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0);
	
	vec4 viewPosition = projMat * modelViewMat * displaced;
	mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
	
    worldPos = (modelMat * displaced).xyz;
	//texCoord = textureCoord;
	normal = (modelMat * vec4(vertexNormal, 0.0)).xyz;
	eyeDir = normalize(-viewPosition).xyz;
	TBN = mat3(normalMatrix * vertexTangent, normalMatrix * vertexBitangent, normalMatrix * vertexNormal);

	//gl_Position = viewPosition;
	gl_Position = MVP * displaced;
	//gl_Position = displaced;
}