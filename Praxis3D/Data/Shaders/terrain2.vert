#version 400 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
//layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
//layout(location = 3) in vec3 vertexTangent;
//layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
//out vec3 worldPos;
out vec2 texCoord;
//out vec3 normal;
//out vec3 eyeDir;
//out mat3 TBN;

//uniform mat4 MVP;
//uniform mat4 modelMat;
//uniform mat4 modelViewMat;
//uniform mat4 projMat;

//uniform sampler2D diffuseTexture;
//uniform sampler2D heightTexture;

//in vec4 Position;
//in vec4 Position;
out vec3 vPosition;

void main()
{
	//texCoord = textureCoord;
	texCoord = vertexPosition.xy;
    vPosition = vertexPosition.xyz;
}