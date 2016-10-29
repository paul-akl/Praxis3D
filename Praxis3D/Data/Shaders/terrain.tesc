#version 400 core

// Produce 4 vertices of output
layout(vertices = 4) out;
	
uniform mat4 MVP;
uniform vec2 screenSize;

float LODFactor = 1.0;

// Transforms a vertex from worls space to normal space
vec4 toNormalSpace(vec4 p_vertex)
{
	vec4 resultVec = MVP * p_vertex;
	resultVec /= resultVec.w;
	return resultVec;
}

// Transforms a vertex from normal space to screen space
vec2 toScreenSpace(vec4 p_vertex)
{
	return (clamp(p_vertex.xy, -1.3, 1.3) + 1) * (screenSize * 0.5);
}

// Checks if a vertex is outside of view furstum
bool isOffscreen(vec4 p_vertex)
{
	if(p_vertex.z < 0.5)
	{
		return true;
	}
	
	return any(lessThan(p_vertex.xy, vec2(-1.7)) || greaterThan(p_vertex.xy, vec2(1.7)));
}

// Calculate LOD of an edge, by using distance between two points in screen space
float calcLevel(vec2 p_ver0, vec2 p_vert1)
{
	return clamp(distance(p_ver0, p_vert1) / LODFactor, 1, 64);
}

void main(void)
{
	//gl_TessLevelInner[0] = 0;
	//gl_TessLevelInner[1] = 0;
	
	//gl_TessLevelOuter[0] = 0;
	//gl_TessLevelOuter[1] = 0;
	//gl_TessLevelOuter[2] = 0;
	//gl_TessLevelOuter[3] = 0;
	
	if(gl_InvocationID == 0)
	{
		vec4 vert0 = toNormalSpace(gl_in[0].gl_Position);
		vec4 vert1 = toNormalSpace(gl_in[1].gl_Position);
		vec4 vert2 = toNormalSpace(gl_in[2].gl_Position);
		vec4 vert3 = toNormalSpace(gl_in[3].gl_Position);
		
		// If all of the vertices are offscreen, set the inner and outer tessellation levels to 0
		if(all(bvec4(isOffscreen(vert0), isOffscreen(vert1), isOffscreen(vert2), isOffscreen(vert3))))
		{
			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
			
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelOuter[3] = 0;
		}
		else
		{
			vec2 screeSpaceVert0 = toScreenSpace(vert0);
			vec2 screeSpaceVert1 = toScreenSpace(vert1);
			vec2 screeSpaceVert2 = toScreenSpace(vert2);
			vec2 screeSpaceVert3 = toScreenSpace(vert3);
			
			float edgeLevelVert0 = calcLevel(screeSpaceVert1, screeSpaceVert2);
			float edgeLevelVert1 = calcLevel(screeSpaceVert0, screeSpaceVert1);
			float edgeLevelVert2 = calcLevel(screeSpaceVert3, screeSpaceVert0);
			float edgeLevelVert3 = calcLevel(screeSpaceVert2, screeSpaceVert3);
			
			gl_TessLevelInner[0] = mix(edgeLevelVert1, edgeLevelVert2, 0.5);
			gl_TessLevelInner[1] = mix(edgeLevelVert0, edgeLevelVert3, 0.5);
			
			gl_TessLevelOuter[0] = edgeLevelVert0;
			gl_TessLevelOuter[1] = edgeLevelVert1;
			gl_TessLevelOuter[2] = edgeLevelVert2;
			gl_TessLevelOuter[3] = edgeLevelVert3;
		}
	}
}