#version 430 core

// Discard fragments based on transparency
#define ALPHA_DISCARD 1

// Number of shadow mapping cascades, determines the amount of layers to render to
#define NUM_OF_CASCADES 5
#define NUM_OF_VERTICES 3

layout(triangles, invocations = NUM_OF_CASCADES) in;
layout(triangle_strip, max_vertices = NUM_OF_VERTICES) out;

struct CascadedShadowMapDataSet
{
	mat4 m_lightSpaceMatrix;
	
	float m_cascadeplanedistance;
	float m_maxBias;
	float m_poissonSampleScale;
	float m_penumbraScale;
};

layout (std140) uniform CSMDataSetBuffer
{
	CascadedShadowMapDataSet m_csmDataSet[NUM_OF_CASCADES];
};

#if ALPHA_DISCARD
in vec2 v_texCoord[];
out vec2 g_texCoord;
#endif

void main()
{
	// Go over each vertex
	for (int i = 0; i < NUM_OF_VERTICES; ++i)
	{
		// Transform the position to light-view space of the corresponding cascade (layer)
		gl_Position = m_csmDataSet[gl_InvocationID].m_lightSpaceMatrix * gl_in[i].gl_Position;
		
		// Set the layer number
		gl_Layer = gl_InvocationID;
		
#if ALPHA_DISCARD
		g_texCoord = v_texCoord[i];
#endif
		EmitVertex();
	}
	EndPrimitive();
}  