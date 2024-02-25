/*
	Geometry pass shader, vertex (geometryPass.vert)
	Geometry stage of the deferred rendering, draws scene objects to fill the geometry buffers:
		position data,
		albedo (diffuse) color,
		normal data,
		emissive color,
		combined material data (RMHA - roughness, metalness, height, ambien-occlusion)
	Reconstructs the XYZ normal from a compressed normal texture holding XY.
	Performs Parallax Occlusion Mapping, if defined.
	Performs alpha discard, if defined.
*/
#version 430 core

#define NUM_OF_MATERIAL_TYPES 4
#define MATERIAL_TYPE_DIFFUSE 0
#define MATERIAL_TYPE_NORMAL 1
#define MATERIAL_TYPE_EMISSIVE 2
#define MATERIAL_TYPE_COMBINED 3

#define PARALLAX_MAPPING 0

struct MaterialData
{
	vec4 m_color;
	vec2 m_scale;
	vec2 m_framing;
};

layout (std140) uniform MaterialDataBuffer
{
	MaterialData m_materialData[NUM_OF_MATERIAL_TYPES];
};

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

// Variables passed to fragment shader
out mat3 TBN;
out mat3 normalMatrix;
out vec2 texCoord[NUM_OF_MATERIAL_TYPES];
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
	for(int i = 0; i < NUM_OF_MATERIAL_TYPES; i++)
	{
		texCoord[i] = (textureCoord + m_materialData[i].m_framing) * m_materialData[i].m_scale;
	}
		
	// Compute TBN matrix components
	const vec3 T = normalize(normalMatrix * vertexTangent);
    const vec3 B = normalize(normalMatrix * vertexBitangent);
    const vec3 N = normalize(normalMatrix * vertexNormal);
	
	// Compute TBN matrix
    TBN = (mat3(T, B, N));
	
#if PARALLAX_MAPPING
	// Calculate TBN needed for tangent space (used for parallax mapping)
	const mat3 tangentTBN = transpose(TBN);
	
	// Calculate variables needed for parallax mapping
    tangentCameraPos = tangentTBN * cameraPosVec;
    tangentFragPos  = tangentTBN * fragPos;
#endif
	
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}