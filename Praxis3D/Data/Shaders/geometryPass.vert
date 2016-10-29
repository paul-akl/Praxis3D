#version 330 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out vec3 fragPos;
out vec2 texCoord;
out vec3 normal;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;
uniform float textureTilingFactor;

void main(void)
{		
	// Multiply position and normal by model matrix (to convert them into world space)
    fragPos = vec3(modelMat * vec4(vertexPosition, 1.0));
	normal = normalize(vec3(modelMat * vec4(vertexNormal, 0.0)));
	
	// Multiply texture coordinates by the tiling factor. The higher the factor, the denser the tiling
	texCoord = textureCoord * textureTilingFactor;
	
	// Compute TBN matrix (method 1)
	mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
	TBN = mat3(normalMatrix * vertexTangent, normalMatrix * vertexBitangent, normalMatrix * vertexNormal);
	
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}