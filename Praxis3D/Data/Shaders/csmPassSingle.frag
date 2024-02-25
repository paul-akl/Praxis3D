#version 430 core

#define NUM_OF_MATERIAL_TYPES 4
#define MATERIAL_TYPE_DIFFUSE 0
#define MATERIAL_TYPE_NORMAL 1
#define MATERIAL_TYPE_EMISSIVE 2
#define MATERIAL_TYPE_COMBINED 3

// Discard fragments based on transparency
#define ALPHA_DISCARD 1

#if ALPHA_DISCARD
uniform sampler2D diffuseTexture;
uniform float alphaThreshold;

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

in vec2 texCoord;
#endif

void main()
{
#if ALPHA_DISCARD

	// Get fragment alpha value
	float alpha = texture(diffuseTexture, texCoord).a * m_materialData[MATERIAL_TYPE_DIFFUSE].m_color.a;
	
	// Discard fragment if the diffuse alpha color value is smaller than alpha threshold
	if(alpha < alphaThreshold)
		discard;

#endif
}