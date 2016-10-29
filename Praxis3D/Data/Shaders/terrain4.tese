#version 410 core

layout(triangles, fractional_odd_spacing, cw) in;

//uniform mat4 MVP;
//uniform mat4 Modelview;

in vec3 worldPos_CTRL[];
in vec2 texCoord_CTRL[];
in vec3 normal_CTRL[];
in vec3 eyeDir_CTRL[];
in mat3 TBN_CTRL[];

in vec3 tangent_CTRL[];
in vec3 bitangent_CTRL[];

out vec3 worldPos_TE;
out vec2 texCoord_TE;
out vec3 normal_TE;
out vec3 eyeDir_TE;
out mat3 TBN_TE;

out vec3 tangent_TE;
out vec3 bitangent_TE;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform mat4 viewProjMat;

uniform mat4 modelMat;
uniform mat4 projMat;

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
	worldPos_TE = interpolate(worldPos_CTRL[0], worldPos_CTRL[1], worldPos_CTRL[2]);
	texCoord_TE = interpolate(texCoord_CTRL[0], texCoord_CTRL[1], texCoord_CTRL[2]);
	//vec4 test = modelMat * vec4(worldPos_TE, 1.0);
	//texCoord_TE = vec2(test.x, test.z) * 0.000002 + 0.5;
	
	normal_TE = normalize(interpolate(normal_CTRL[0], normal_CTRL[1], normal_CTRL[2]));
	eyeDir_TE = interpolate(eyeDir_CTRL[0], eyeDir_CTRL[1], eyeDir_CTRL[2]);
	//TBN_TE = interpolate(TBN_CTRL[0], TBN_CTRL[1], TBN_CTRL[2]);
	
	tangent_TE = interpolate(tangent_CTRL[0], tangent_CTRL[1], tangent_CTRL[2]);
	bitangent_TE = interpolate(bitangent_CTRL[0], bitangent_CTRL[1], bitangent_CTRL[2]);

	float height = texture(normalTexture, texCoord_TE).a;
	//worldPos_TE.y += height * 200.0;
	
	float TEXHEIGHT = 2048.0;
	float TEXWIDTH = 2048.0;
	
	float h0 = texture(normalTexture, texCoord_TE + vec2(1/TEXWIDTH, 0.0)).a;
	float h1 = texture(normalTexture, texCoord_TE - vec2(1/TEXWIDTH, 0.0)).a;
	float h2 = texture(normalTexture, texCoord_TE + vec2(0.00, 1/TEXHEIGHT)).a;
	float h3 = texture(normalTexture, texCoord_TE - vec2(0.00, 1/TEXHEIGHT)).a;
	
	float hh0 = texture(normalTexture, texCoord_CTRL[0]).a;
	float hh1 = texture(normalTexture, texCoord_CTRL[1]).a;
	float hh2= texture(normalTexture, texCoord_CTRL[2]).a;
	
	const ivec3 off2 = ivec3(-1, 0, 1);
	float s01 = textureOffset(normalTexture, texCoord_TE, off2.xy).a;
	float s21 = textureOffset(normalTexture, texCoord_TE, off2.zy).a;
	float s10 = textureOffset(normalTexture, texCoord_TE, off2.yx).a;
	float s12 = textureOffset(normalTexture, texCoord_TE, off2.yz).a;
	
	vec3 pos0 = vec3(worldPos_TE.x, worldPos_TE.y + h0 * 200.0, worldPos_TE.z);
	vec3 pos1 = vec3(worldPos_TE.x, worldPos_TE.y + h1 * 200.0, worldPos_TE.z);
	vec3 pos2 = vec3(worldPos_TE.x, worldPos_TE.y + h2 * 200.0, worldPos_TE.z);
	vec3 pos3 = vec3(worldPos_TE.x, worldPos_TE.y + h3 * 200.0, worldPos_TE.z);
	
	vec3 posss0 = worldPos_TE + normal_TE * h0 * 1.0;// - vec3(0.0, 0.0, 0.5);
	vec3 posss1 = worldPos_TE + normal_TE * h1 * 1.0;// + vec3(0.0, 0.0, 0.5);
	vec3 posss2 = worldPos_TE + normal_TE * h2 * 1.0;// - vec3(0.5, 0.0, 0.0);
	vec3 posss3 = worldPos_TE + normal_TE * h3 * 1.0;// + vec3(0.5, 0.0, 0.0);
	
	vec3 poss0 = vec3(worldPos_CTRL[0].x, worldPos_CTRL[0].y + hh0 * 200.0, worldPos_CTRL[0].z);
	vec3 poss1 = vec3(worldPos_CTRL[1].x, worldPos_CTRL[1].y + hh1 * 200.0, worldPos_CTRL[1].z);
	vec3 poss2 = vec3(worldPos_CTRL[2].x, worldPos_CTRL[2].y + hh2 * 200.0, worldPos_CTRL[2].z);
	
	vec3 wpos0 = (worldPos_TE + vec3(-1.0, 0.0, 0.0)) + (normal_TE * s01 * 200.0);
	vec3 wpos1 = (worldPos_TE + vec3(1.0, 0.0, 0.0)) + (normal_TE * s10 * 200.0);
	vec3 wpos2 = (worldPos_TE + vec3(0.0, 0.0, 1.0)) + (normal_TE * s21 * 200.0);
	vec3 wpos3 = (worldPos_TE + vec3(0.0, 0.0, -1.0)) + (normal_TE * s12 * 200.0);
	
	//float SCALE = displace_ratio;
	worldPos_TE += normal_TE * height * 200.0;
	gl_Position = viewProjMat * vec4(worldPos_TE, 1.0);

	vec2 uv =  texCoord_TE;
	vec2 du = vec2(1/TEXWIDTH, 0);
	vec2 dv = vec2(0, 1/TEXHEIGHT);
	float dhdu = (2/TEXWIDTH) * (texture(normalTexture, uv+du).a - texture(normalTexture, uv-du).a);
	float dhdv = (2/TEXHEIGHT) * (texture(normalTexture, uv+dv).a - texture(normalTexture, uv-dv).a);
	//normal_TE = normalize(normal_TE + tangent_TE * dhdu + bitangent_TE * dhdv);

	//vec3 tangent_TE = vec3(0.0, 0.0, 1.0);
	//vec3 bitangent_TE = cross(tangent_TE, normal_TE);
	//tangent_TE = cross(normal_TE, bitangent_TE);
	
	
	
	//normal_TE = normalize(cross(wpos1 - wpos0, wpos2 - wpos0));
	normal_TE = normalize(cross(normalize(wpos0 - wpos1), normalize(wpos2 - wpos3)));
	//normal_TE = wpos0 - wpos1;
	normal_TE = vec3(s01 - s10) * 100.0;
		
	//normal_TE = normalize(cross(pos0 + pos1, pos2 + pos3));
	
	//normal_TE = normalize(cross(poss0 * poss1, poss0 * poss2));
	
	//normal_TE = normalize(cross(pos0, pos1));
	
	vec3 norm0 = normalize(cross(pos0, pos1));
	vec3 norm1 = normalize(cross(pos2, pos3));
	
	//normal_TE = normalize(cross(norm0, norm1));
	//normal_TE = norm0;
	
	
	

	// You can actually calculate it without a cross product, 
	// by using the "finite difference method" (or at least I think it is called in this way).

	// Actually it is fast enough that I use it to calculate the normals on the fly in a vertex shader.

	// # P.xy store the position for which we want to calculate the normals
	// # height() here is a function that return the height at a point in the terrain

	// read neightbor heights using an arbitrary small offset
	
	
	//float hL = texture(normalTexture, texCoord_TE - vec2(1/TEXWIDTH, 0.0)).a * 200.0;
	//float hR = texture(normalTexture, texCoord_TE + vec2(1/TEXWIDTH, 0.0)).a * 200.0;
	//float hD = texture(normalTexture, texCoord_TE - vec2(0.0, 1/TEXHEIGHT)).a * 200.0;
	//float hU = texture(normalTexture, texCoord_TE + vec2(0.0, 1/TEXHEIGHT)).a * 200.0;
	
	//height *= 200.0f;
	
	//vec3 off = vec3(1.0, 1.0, 0.0);
	//float hL = height(P.xy - off.xz);
	//float hR = height(P.xy + off.xz);
	//float hD = height(P.xy - off.zy);
	//float hU = height(P.xy + off.zy);

	vec3 off = vec3(0.01, 0.01, 0.0);
	float hL = texture(normalTexture, texCoord_TE - off.xz).a * 200.0;
	float hR = texture(normalTexture, texCoord_TE + off.xz).a * 200.0;
	float hD = texture(normalTexture, texCoord_TE - off.zy).a * 200.0;
	float hU = texture(normalTexture, texCoord_TE + off.zy).a * 200.0;
	
	// deduce terrain normal_TE
	vec3 N = vec3(0.0, 1.0, 0.0);
	N.x = hL - hR;
	N.y = hD - hU;
	N.z = 2.0;
	//normal_TE = normalize(N);

	//Returns a normal_TE from a grid of heights
	//float3 computeNormals( float h_A, float h_B, float h_C, float h_D, float h_N, float heightScale )
	//{
		//To make it easier we offset the points such that n is "0" height
		vec3 va = vec3(0.0, 1.0, (hU - height));
		vec3 vb = vec3(1.0, 0.0, (hR - height));
		vec3 vc = vec3(0.0, -1.0, (hD - height));
		vec3 vd = vec3(-1.0, 0.0, (hL - height));
		
		//cross products of each vector yields the normal_TE of each tri - return the average normal_TE of all 4 tris
		vec3 average_n = ( cross(va, vb) + cross(vb, vc) + cross(vc, vd) + cross(vd, va) ) / -4;
		//normal_TE = normalize(average_n);
		//return normalize( average_n );
	//}
	
	//normal_TE = normalize(( cross(posss0, posss1) + cross(posss1, posss2) + cross(posss2, posss3) + cross(posss3, posss0) ) / -4);
	
	//tangent_TE = vec3(0.0, 0.0, 1.0);
	//bitangent_TE = cross(tangent_TE, normal_TE);
	//tangent_TE = cross(normal_TE, bitangent_TE);
	
	mat3 normalMat = transpose(inverse(mat3(modelMat)));
	TBN_TE = mat3(normalMat * tangent_TE, normalMat * bitangent_TE, normalMat * normal_TE);
	
	//vec3 normm0 = normalize(cross(hL, hR));
	//vec3 normm1 = normalize(cross(hD, hU));
	
	//normal_TE = normalize(cross(normm0, normm1));
	
	// Dir = (B - A) x (C - A)
	// Norm = Dir / len(Dir)
	
    //worldPos_TE = gl_Position.xyz;
	//gl_Position = projMat * vec4(worldPos_TE, 1.0);
	
    //vec3 p0 = gl_TessCoord.x * tcPosition[0];
    //vec3 p1 = gl_TessCoord.y * tcPosition[1];
    //vec3 p2 = gl_TessCoord.z * tcPosition[2];
	
    //tePatchDistance = gl_TessCoord;
    //tePosition = normalize(p0 + p1 + p2);
	
    //gl_Position = MVP * vec4(tePosition, 1);
}