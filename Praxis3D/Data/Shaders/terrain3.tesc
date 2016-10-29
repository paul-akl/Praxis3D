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

out vec3 worldPos_CTRL[];
out vec2 texCoord_CTRL[];
out vec3 normal_CTRL[];
out vec3 eyeDir_CTRL[];
out mat3 TBN_CTRL[];

//in vec2 posV[];

//out vec2 posTC[];

uniform mat4 MVP;
uniform mat4 modelViewMat;
uniform vec3 cameraPosVec;
uniform ivec2 screenSize;

uniform sampler2D diffuseTexture;

float TerrainHeightOffset = 100.0;
float patchSize = 1.0;
int scaleFactor = 2;
int pixelsPerEdge = 1;
float gridSpacing = 160.0;
float heightStep = 0.1;

float height(float u, float v)
{

	return (texture(diffuseTexture, vec2(u,v)).a  * heightStep);
}

// Checks if the segment between two vector points is at least partially inside the view frustum
bool insideViewFrustum(vec4 p_vec0, vec4 p_vec1)
{
	if ((p_vec0.x < -p_vec0.w && p_vec1.x < -p_vec1.w) || (p_vec0.x > p_vec0.w && p_vec1.x > p_vec1.w) ||
		(p_vec0.z < -p_vec0.w && p_vec1.z < -p_vec1.w) || (p_vec0.z > p_vec0.w && p_vec1.z > p_vec1.w))
		return false;
	else
		return true;
}

// Calculates a sphere size in screen space of the two vector points
float screenSphereSize(vec4 p_vec0, vec4 p_vec1)
{
	vec4 viewCenter = (p_vec0 + p_vec1) * 0.5;
	vec4 viewUp = viewCenter;
	viewUp.y += distance(p_vec0, p_vec1);
	vec4 vec0Proj = viewCenter;
	vec4 vec1Proj = viewUp;
	
	// Get normalized deviced coordinates
	vec4 vec0NDC, vec1NDC;
	vec0NDC = vec0Proj/vec0Proj.w;
	vec1NDC = vec1Proj/vec1Proj.w;
	
	return(clamp(length((vec1NDC.xy - vec0NDC.xy) * screenSize * 0.5) / (pixelsPerEdge), 1.0, patchSize));
}

void main()
{
	int scaleF;
	if (scaleFactor == 0)
		scaleF = 1;
	else
		scaleF = scaleFactor;

	vec2 iLevel;
	vec4 oLevel;

	vec4 posTransV[4];
	vec2 pAux;
	vec2 posTCAux[4];

	ivec2 tSize = textureSize(diffuseTexture, 0) * scaleF;// * 2;
	float div = patchSize / tSize.x;

	//posTC[gl_InvocationID] = posV[gl_InvocationID];

	vec2 position0 = vec2(worldPos[0].x, worldPos[0].z);
	
	posTCAux[0] = position0;
	posTCAux[1] = position0 + vec2(0.0, div);
	posTCAux[2] = position0 + vec2(div,0.0);
	posTCAux[3] = position0 + vec2(div,div);
	
	
	pAux = posTCAux[0] * tSize * gridSpacing;
	posTransV[0] = MVP * vec4(pAux[0], height(posTCAux[0].x,posTCAux[0].y), pAux[1], 1.0);

	pAux = posTCAux[1] * tSize * gridSpacing;
	posTransV[1] = MVP * vec4(pAux[0], height(posTCAux[1].x,posTCAux[1].y), pAux[1], 1.0);

	pAux = posTCAux[2] * tSize * gridSpacing;
	posTransV[2] = MVP * vec4(pAux[0], height(posTCAux[2].x,posTCAux[2].y), pAux[1], 1.0);

	pAux = posTCAux[3] * tSize * gridSpacing;
	posTransV[3] = MVP * vec4(pAux[0], height(posTCAux[3].x,posTCAux[3].y), pAux[1], 1.0);
	
			
				
	// Screen size based LOD
	oLevel = vec4(screenSphereSize(posTransV[gl_InvocationID], posTransV[gl_InvocationID+1]),
				screenSphereSize(posTransV[gl_InvocationID+0], posTransV[gl_InvocationID+2]),
				screenSphereSize(posTransV[gl_InvocationID+2], posTransV[gl_InvocationID+3]),
				screenSphereSize(posTransV[gl_InvocationID+3], posTransV[gl_InvocationID+1]));
	iLevel = vec2(max(oLevel[1] , oLevel[3]) , max(oLevel[0] , oLevel[2]) );
		
	//gl_TessLevelOuter[0] = oLevel[0];
	//gl_TessLevelOuter[1] = oLevel[1];
//gl_TessLevelOuter[2] = oLevel[2];
	//gl_TessLevelOuter[3] = oLevel[3];
	//gl_TessLevelInner[0] = iLevel[0];
//gl_TessLevelInner[1] = iLevel[1];
	
	gl_TessLevelOuter[0] = 64;
	gl_TessLevelOuter[1] = 64;
	gl_TessLevelOuter[2] = 64;
	gl_TessLevelOuter[3] = 64;
	gl_TessLevelInner[0] = 64;
	gl_TessLevelInner[1] = 64;
	
	worldPos_CTRL[gl_InvocationID] = worldPos[gl_InvocationID];
	texCoord_CTRL[gl_InvocationID] = texCoord[gl_InvocationID];
	normal_CTRL[gl_InvocationID] = normal[gl_InvocationID];
	eyeDir_CTRL[gl_InvocationID] = eyeDir[gl_InvocationID];
	TBN_CTRL[gl_InvocationID] = TBN[gl_InvocationID];
	
	/*
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
	gl_TessLevelOuter[0] = calcDistance(gl_in[3].gl_Position, gl_in[0].gl_Position, texCoord[3], texCoord[0]);
	gl_TessLevelOuter[1] = calcDistance(gl_in[0].gl_Position, gl_in[1].gl_Position, texCoord[0], texCoord[1]);
	gl_TessLevelOuter[2] = calcDistance(gl_in[1].gl_Position, gl_in[2].gl_Position, texCoord[1], texCoord[2]);
	gl_TessLevelOuter[3] = calcDistance(gl_in[2].gl_Position, gl_in[3].gl_Position, texCoord[2], texCoord[3]);
	gl_TessLevelInner[0] = gl_TessLevelOuter[2];*/
}

