#version 410 core

layout(triangles, fractional_odd_spacing, cw) in;

//uniform mat4 MVP;
//uniform mat4 Modelview;

in vec3 worldPos_CTRL[];
in vec2 texCoord_CTRL[];
in vec3 normal_CTRL[];
in vec3 eyeDir_CTRL[];
in mat3 TBN_CTRL[];

out vec3 worldPos;
out vec2 texCoord;
out vec3 normal;
out vec3 eyeDir;
out mat3 TBN;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform mat4 MVP;
uniform mat4 viewProjMat;

uniform mat4 modelMat;
uniform mat4 projMat;

float TerrainHeightOffset = 100.0;
float patchSize = 100.0;
int scaleFactor = 2;
int pixelsPerEdge = 1;
float gridSpacing = 160.0;
float heightStep = 0.1;

#define sizeFactor 1.0 / patchSize

float height(float u, float v)
{
	return (texture(diffuseTexture, vec2(u,v)).a  * heightStep);
}

vec2 interpolate(vec2 p_vec0, vec2 p_vec1, vec2 p_vec2)
{
	return vec2(gl_TessCoord.x) * p_vec0 + vec2(gl_TessCoord.y) * p_vec1 + vec2(gl_TessCoord.z) * p_vec2;
}

vec3 interpolate(vec3 p_vec0, vec3 p_vec1, vec3 p_vec2)
{
	return vec3(gl_TessCoord.x) * p_vec0 + vec3(gl_TessCoord.y) * p_vec1 + vec3(gl_TessCoord.z) * p_vec2;
}

void main()
{
	vec2 position0 = vec2(worldPos_CTRL[0].x, worldPos_CTRL[0].z);

	ivec2 tSize = textureSize(diffuseTexture, 0) * scaleFactor;
	float div = tSize.x * sizeFactor;

	// Compute texture coordinates
//	uvTE.s = (posTC[0].x + uv.s/div) ;
//	uvTE.t = (posTC[0].y + uv.t/div) ;
	//texCoord = position0.xy + gl_TessCoord.st/div;

	// compute vertex position [0 .. tSize * gridSpacing]
	vec4 res;
	res.x = texCoord.s * tSize.x * gridSpacing;
	res.z = texCoord.t * tSize.y * gridSpacing;
	res.y = height(texCoord.s, texCoord.t);
	res.w = 1.0;
	

	worldPos = interpolate(worldPos_CTRL[0], worldPos_CTRL[1], worldPos_CTRL[2]);
	//texCoord = interpolate(texCoord_CTRL[0], texCoord_CTRL[1], texCoord_CTRL[2]);
	//vec4 test = viewProjMat * vec4(worldPos, 1.0);
	texCoord = vec2(worldPos.x, worldPos.z) * 0.0003 + 0.5;
	normal = normalize(interpolate(normal_CTRL[0], normal_CTRL[1], normal_CTRL[2]));
	eyeDir = interpolate(eyeDir_CTRL[0], eyeDir_CTRL[1], eyeDir_CTRL[2]);
	//TBN = interpolate(TBN_CTRL[0], TBN_CTRL[1], TBN_CTRL[2]);

	//float height = texture(diffuseTexture, texCoord).a;
	//worldPos += normal * height * 3000.0;
	worldPos.y += texture(diffuseTexture, texCoord).a * 10000.0;;
	
	//gl_Position = viewProjMat * res;
	gl_Position = viewProjMat * vec4(worldPos, 1.0);
	//gl_Position = projMat * vec4(worldPos, 1.0);
	
    //vec3 p0 = gl_TessCoord.x * tcPosition[0];
    //vec3 p1 = gl_TessCoord.y * tcPosition[1];
    //vec3 p2 = gl_TessCoord.z * tcPosition[2];
	
    //tePatchDistance = gl_TessCoord;
    //tePosition = normalize(p0 + p1 + p2);
	
    //gl_Position = MVP * vec4(tePosition, 1);
}