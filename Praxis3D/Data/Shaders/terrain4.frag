#version 410 core

// Some drivers require the following
precision highp float;

// Geometry buffers
layout(location = 0) out vec4 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec4 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;

// Variables from vertex shader
in vec3 worldPos;
in vec2 texCoord;
in vec3 normal;
in vec3 eyeDir;
in mat3 TBN;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D glossTexture;
uniform sampler2D heightTexture;

void main(void)
{
	//emissiveBuffer	= normal;

	//vec4 emissiveTexture = texture(emissiveTexture, texCoord).rgba;
	//if(emissiveTexture.a < 0.5)
	//{
	//	emissiveTexture = vec4(0.0);
	//}
	//emissiveBuffer	= emissiveTexture * emissiveTexture.a;
	//diffuseBuffer	= texture(diffuseTexture, texCoord).rgba;
	//texCoordBuffer	= vec4(texCoord, 0.0, 0.0);
	//positionBuffer	= vec4(worldPos, 0.0);
	//normalBuffer	= vec4(normalize(normal), 0.0);
	
	//vec2 newTexCoord = vec2(worldPos.x, worldPos.z);
	vec2 newCoords = texCoord;
	
	//float height = texture(heightTexture, texCoord.st).r;
	//float v = height * 0.04 - 0.02;
	//vec2 newCoords = texCoord+(eyeDir.xy*v);
	
	float specularPower = texture(specularTexture, newCoords).r;
	vec4 emissiveColor = texture(emissiveTexture, newCoords).rgba;
	if(emissiveColor.a < 0.3)
	{
		emissiveColor = vec4(0.0);
	}
	emissiveBuffer	= emissiveColor * emissiveColor.a;

	diffuseBuffer	= vec4(texture(diffuseTexture, newCoords).rgb, 0.0);	// Write the color from model's texture
	positionBuffer	= vec4(worldPos, 0.0);						// Write fragment's position in world space
	normalBuffer = vec4(TBN * normalize((2.0 * (texture(normalTexture, newCoords).rgb) - 1.0)), specularPower);
}
