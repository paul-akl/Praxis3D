#version 430 core

#define PARALLAX_MAPPING 0

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out mat3 TBN;
out mat3 normalMatrix;
out vec2 texCoord;
out vec3 fragPos;
out vec3 normal;
out vec3 tangentFragPos;
out vec3 tangentCameraPos;
out float parallaxScale;
out float parallaxLOD;

uniform mat4 MVP;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 modelViewMat;
uniform mat4 projMat;
uniform mat4 viewProjMat;
uniform vec3 cameraPosVec;
uniform float textureTilingFactor;
uniform float heightScale;
uniform float parallaxMappingLOD;

void main(void)
{		
	// Multiply position by model matrix (to convert it into world space)
    fragPos = vec3(modelMat * vec4(vertexPosition, 1.0));
	
	// Calculate normal matrix and convert normal into world space
    normalMatrix = transpose(inverse(mat3(modelMat)));
    normal = normalMatrix * vertexNormal;
	
	// Square the parallax LOD distance, so there's no need to do that in the fragment shader
	parallaxLOD = parallaxMappingLOD * parallaxMappingLOD;
	
	// Copy the height scale value
	parallaxScale = heightScale;
	
	// Multiply texture coordinates by the tiling factor. The higher the factor, the denser the tiling
	texCoord = textureCoord * textureTilingFactor;
		
	// Compute TBN matrix components
	vec3 T = normalize(normalMatrix * vertexTangent);
    vec3 B = normalize(normalMatrix * vertexBitangent);
    vec3 N = normalize(normalMatrix * vertexNormal);
	
	// Compute TBN matrix
    TBN = (mat3(T, B, N));
	
#if PARALLAX_MAPPING
	// Calculate TBN needed for tangent space (used for parallax mapping)
	mat3 tangentTBN = transpose(TBN);
	
	// Calculate variables needed for parallax mapping
    tangentCameraPos = tangentTBN * cameraPosVec;
    tangentFragPos  = tangentTBN * fragPos;
#endif
	
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}