#version 400 core

// Produce 4 vertices of output
//layout(vertices = 4) out;
	
//uniform mat4 MVP;
//uniform vec2 screenSize;

//float LODFactor = 1.0;

layout(vertices = 3) out;

in vec3 vPosition[];
out vec3 tcPosition[];

#define ID gl_InvocationID

void main()
{
	float TessLevelInner = 64;
	float TessLevelOuter = 64;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    if (ID == 0) 
	{
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
}