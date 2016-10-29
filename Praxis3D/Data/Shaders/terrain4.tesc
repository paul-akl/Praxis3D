#version 410 core

layout(vertices = 3) out;

//in vec3 vPosition[];
//out vec3 tcPosition[];

//#define ID gl_InvocationID

in vec3 worldPos[];
in vec2 texCoord[];
in vec3 normal[];
in vec3 eyeDir[];
in mat3 TBN[];

in vec3 tangent[];
in vec3 bitangent[];

out vec3 worldPos_CTRL[];
out vec2 texCoord_CTRL[];
out vec3 normal_CTRL[];
out vec3 eyeDir_CTRL[];
out mat3 TBN_CTRL[];

out vec3 tangent_CTRL[];
out vec3 bitangent_CTRL[];

//in vec2 posV[];

//out vec2 posTC[];

uniform mat4 modelViewMat;
uniform vec3 cameraPosVec;

uniform sampler2D diffuseTexture;

float TerrainHeightOffset = 100.0;
float patchSize = 10.0;

float calcTessLevel(float p_distance0, float p_distance1)
{
	float averageDistance = (p_distance0 + p_distance1) / 2.0;
		
	if(averageDistance <= 20.0)
	{
		return 10.0;
	}
		
	if(averageDistance <= 200.0)
	{
		return 7.0;
	}
		
		return 1.0;
}

float calcDistance(vec4 p0, vec4 p1, vec2 t0, vec2 t1)
{
	float samp = texture(diffuseTexture, t0).a;
	p0.y = samp * TerrainHeightOffset;
	samp = texture(diffuseTexture, t1).a;
	p1.y = samp * TerrainHeightOffset;

	vec4 view0 = modelViewMat * p0;
	vec4 view1 = modelViewMat * p1;

	float MinDepth = 10.0;
	float MaxDepth = 100000.0;

	float d0 = clamp( (abs(p0.z) - MinDepth) / (MaxDepth - MinDepth), 0.0, 1.0);
	float d1 = clamp( (abs(p1.z) - MinDepth) / (MaxDepth - MinDepth), 0.0, 1.0);

	float t = mix(64, 2, (d0 + d1) * 0.5);

	if (t <= 2.0)
	{ 
		return 2.0;
	}
	if (t <= 4.0)
	{ 
		return 4.0;
	}
	if (t <= 8.0)
	{ 
		return 8.0;
	}
	if (t <= 16.0)
	{ 
		return 16.0;
	}
	if (t <= 32.0)
	{ 
		return 32.0;
	}
	
	return 20.0;
}

void main()
{	
	worldPos_CTRL[gl_InvocationID] = worldPos[gl_InvocationID];
	texCoord_CTRL[gl_InvocationID] = texCoord[gl_InvocationID];
	normal_CTRL[gl_InvocationID] = normal[gl_InvocationID];
	eyeDir_CTRL[gl_InvocationID] = eyeDir[gl_InvocationID];
	TBN_CTRL[gl_InvocationID] = TBN[gl_InvocationID];
	
	tangent_CTRL[gl_InvocationID] = tangent[gl_InvocationID];
	bitangent_CTRL[gl_InvocationID] = bitangent[gl_InvocationID];

	//float TessLevelInner = 64;
	//float TessLevelOuter = 64;
    //tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
	
	float distanceVert0 = distance(cameraPosVec, worldPos_CTRL[0]);
	float distanceVert1 = distance(cameraPosVec, worldPos_CTRL[1]);
	float distanceVert2 = distance(cameraPosVec, worldPos_CTRL[2]);
	
	//gl_TessLevelOuter[0] = calcTessLevel(distanceVert1, distanceVert2);
	//gl_TessLevelOuter[1] = calcTessLevel(distanceVert2, distanceVert0);
	//gl_TessLevelOuter[2] = calcTessLevel(distanceVert0, distanceVert1);
	//gl_TessLevelInner[0] = gl_TessLevelOuter[2];
	
		// Outer tessellation level
	//gl_TessLevelOuter[0] = calcDistance(gl_in[3].gl_Position, gl_in[0].gl_Position, texCoord[3], texCoord[0]);
	//gl_TessLevelOuter[1] = calcDistance(gl_in[0].gl_Position, gl_in[1].gl_Position, texCoord[0], texCoord[1]);
	//gl_TessLevelOuter[2] = calcDistance(gl_in[1].gl_Position, gl_in[2].gl_Position, texCoord[1], texCoord[2]);
	//gl_TessLevelOuter[3] = calcDistance(gl_in[2].gl_Position, gl_in[3].gl_Position, texCoord[2], texCoord[3]);
	//gl_TessLevelInner[0] = gl_TessLevelOuter[2];
	
	gl_TessLevelOuter[0] = 7.0;
	gl_TessLevelOuter[1] = 7.0;
	gl_TessLevelOuter[2] = 7.0;
	gl_TessLevelOuter[3] = 7.0;
	gl_TessLevelInner[0] = 7.0;
}

