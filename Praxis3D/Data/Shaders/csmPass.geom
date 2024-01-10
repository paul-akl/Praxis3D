#version 430 core

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

void main()
{          
	for (int i = 0; i < NUM_OF_VERTICES; ++i)
	{
		gl_Position = m_csmDataSet[gl_InvocationID].m_lightSpaceMatrix * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  