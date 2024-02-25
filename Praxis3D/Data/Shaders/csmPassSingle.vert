#version 430 core

#define NUM_OF_MATERIAL_TYPES 4
#define MATERIAL_TYPE_DIFFUSE 0
#define MATERIAL_TYPE_NORMAL 1
#define MATERIAL_TYPE_EMISSIVE 2
#define MATERIAL_TYPE_COMBINED 3

// Discard fragments based on transparency
#define ALPHA_DISCARD 1

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;

#if ALPHA_DISCARD
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

layout(location = 2) in vec2 textureCoord;

out vec2 texCoord;
#endif

uniform mat4 modelMat;

void main(void) 
{
	gl_Position = modelMat * vec4(vertexPosition, 1.0);
	
#if ALPHA_DISCARD
	texCoord = (textureCoord + m_materialData[MATERIAL_TYPE_DIFFUSE].m_framing) * m_materialData[MATERIAL_TYPE_DIFFUSE].m_scale;
#endif
}