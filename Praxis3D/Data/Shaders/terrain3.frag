#version 410 core

// Some drivers require the following
precision highp float;

// Geometry buffers
layout(location = 0) out vec4 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec4 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;

// Variables from vertex shader
in vec3 worldPos;
in vec2 texCoord;
in vec3 normal;
in vec3 eyeDir;
in mat3 TBN;

in vec3 tangent;
in vec3 bitangent;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D glossTexture;
uniform sampler2D heightTexture;

uniform mat4 modelMat;
uniform mat4 modelViewMat;

void main(void)
{
	//emissiveBuffer	= normal;

	//vec4 emissiveTexture = texture(emissiveTexture, texCoord).rgba;
	//if(emissiveTexture.a < 0.5)
	//{
	//	emissiveTexture = vec4(0.0);
	//}
	//emissiveBuffer	= emissiveTexture * emissiveTexture.a;
	//diffuseBuffer	= texture(diffuseTexture, texCoord).rgba;
	//texCoordBuffer	= vec4(texCoord, 0.0, 0.0);
	//positionBuffer	= vec4(worldPos, 0.0);
	//normalBuffer	= vec4(normalize(normal), 0.0);
	
	//vec2 newTexCoord = vec2(worldPos.x, worldPos.z);
	vec2 newCoords = texCoord;
	
	float height = texture(normalTexture, texCoord).a * 1.0;
	
	vec3 off = vec3(1 / 4096.0, 1 / 4096.0, 0.0);
	float hL = texture(normalTexture, texCoord - off.xz).a;
	float hR = texture(normalTexture, texCoord + off.xz).a;
	float hD = texture(normalTexture, texCoord - off.zy).a;
	float hU = texture(normalTexture, texCoord + off.zy).a;
	
	vec3 vva = vec3(0.0, 1.0, (hU - height) * 1.0);
	vec3 vvb = vec3(1.0, 0.0, (hR - height) * 1.0);
	vec3 vvc = vec3(0.0, -1.0, (hD - height) * 1.0);
	vec3 vvd = vec3(-1.0, 0.0, (hL - height) * 1.0);
	
	//vec3 vva = vec3(0.0, (hU - height), 1.0);
	//vec3 vvb = vec3(1.0, (hR - height), 0.0);
	//vec3 vvc = vec3(0.0, (hD - height), -1.0);
	//vec3 vvd = vec3(-1.0, (hL - height), 0.0);
		
	//cross products of each vector yields the normal of each tri - return the average normal of all 4 tris
	vec3 average_n = ( cross(vva, vvb) + cross(vvb, vvc) + cross(vvc, vvd) + cross(vvd, vva) ) / -4;
	//vec3 normal2 = normalize(average_n);
	
	vec2 uv =  texCoord;
	vec2 du = vec2(1/4096.0, 0);
	vec2 dv = vec2(0, 1/4096.0);
	float dhdu = (2/4096.0) * (texture(normalTexture, uv+du).a - texture(normalTexture, uv-du).a);
	float dhdv = (2/4096.0) * (texture(normalTexture, uv+dv).a - texture(normalTexture, uv-dv).a);
	//vec3 normal2 = normalize(normal + tangent * dhdu + bitangent * dhdv);
	
	vec3 Norm = vec3(0.0, 1.0, 0.0);
	Norm.x = hL - hR;
	Norm.y = hD - hU;
	Norm.z = 1.0;
	//vec3 normal2 = normalize(N);
	
	//ivec2 xy = ivec2(-1, 0);
	//ivec2 zy = ivec2(1, 0);
	//ivec2 yx = ivec2(0, -1);
	//ivec2 yz = ivec2(0, 1);

	vec4 wave = texture(normalTexture, texCoord);
	float s11 = wave.a * 200.0;
	
	vec2 size = vec2(2.0, 0.0);
	const ivec3 off2 = ivec3(-1, 0, 1);
	
	float s01 = textureOffset(normalTexture, texCoord, off2.xy).a * 200.0;
	float s21 = textureOffset(normalTexture, texCoord, off2.zy).a * 200.0;
	float s10 = textureOffset(normalTexture, texCoord, off2.yx).a * 200.0;
	float s12 = textureOffset(normalTexture, texCoord, off2.yz).a * 200.0;
	//vec3 va = normalize(vec3(size.xy, s21 - s01));
	//vec3 vb = normalize(vec3(size.yx, s12 - s10));
	vec3 va = normalize(vec3(size.x, s21-s01, size.y)); 
	vec3 vb = normalize(vec3(size.y, s12-s10, -size.x));
	//vec3 va = normalize(vec3(size.y, s21 - s01, size.x));
	//vec3 vb = normalize(vec3(size.x, s12 - s10, size.y));
	vec4 bump = vec4(cross(va, vb), s11);
	
	vec3 normal2 = bump.xyz;
		
	//normal = normalize(N);
	//mat3 normalMat = transpose(inverse(mat3(modelMatrix)));
	//TBN = mat3(normalMat * VertexTangent, normalMat * TextureBiTangent, normalMat * VertexNormal);
	//normal = (modelMat * vec4(normal, 0.0)).xyz;
	
	mat3 normalMat = transpose(inverse(mat3(modelMat)));
	//mat3 TBN2 = mat3(normalMat * tangent, normalMat * bitangent, normalMat * normal);
	//mat3 TBN2 = mat3(normalMat * tangent, normalMat * bitangent, normalMat * normalize(vec3(0.0, 1.0, 0.0)));
	
	vec3 T = normalize(vec3(modelMat * vec4(tangent,   0.0)));
	vec3 B = normalize(vec3(modelMat * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(modelMat * vec4(normal2,    0.0)));
	mat3 TBN2 = mat3(T, B, N);
	
	float h0 = texture(normalTexture, texCoord + vec2(1 / 4096.0, 0.0)).a;
	float h1 = texture(normalTexture, texCoord - vec2(1 / 4096.0, 0.0)).a;
	
	//mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
	//vec3 normal = (modelMat * vec4(0.0, 1.0, 0.0, 0.0)).xyz;
	//vec3 normal2 = vec3(0.0, 1.0, 0.0);
	//vec3 normal3 = vec3(texture(normalTexture, newCoords).r, texture(normalTexture, newCoords).b, texture(normalTexture, newCoords).g);
	
	//vec3 tangent = vec3(0.0, 0.0, 1.0);
	//vec3 bitangent = cross(tangent, normal);
	//tangent = cross(normal, bitangent);
	
	//normalMatrix = mat3(normalMatrix * tangent, normalMatrix * bitangent, normalMatrix * normal);
	
	//float height = texture(heightTexture, texCoord.st).r;
	//float v = height * 0.04 - 0.02;
	//vec2 newCoords = texCoord+(eyeDir.xy*v);
	
	vec3 normalMap = normalize((2.0 * texture(normalTexture, texCoord).rbg - 1.0));
	//normalMap = normalize(cross(normalMap, normalize(vec3(0.0, 0.0, 1.0))));
	normalMap = normalize(vec4(normalMap, 0.0) * vec4(0.0, -1.0, 0.0, 1.0)).xyz;
	
	float specularPower = texture(specularTexture, newCoords).r;
	vec4 emissiveColor = texture(emissiveTexture, newCoords).rgba;
	if(emissiveColor.a < 0.3)
	{
		emissiveColor = vec4(0.0);
	}
	emissiveBuffer	= emissiveColor * emissiveColor.a;

	diffuseBuffer	= vec4(texture(diffuseTexture, newCoords).rbg, 0.0);	// Write the color from model's texture
	positionBuffer	= vec4(worldPos, 0.0);						// Write fragment's position in world space
	//normalBuffer = vec4(0.0, 1.0, 0.0, 1.0);
	//normalBuffer = vec4(normalize(texture(normalTexture, newCoords).rbg), 0.0);
	//normalBuffer = vec4(normalize(vec3(texture(normalTexture, newCoords).r, texture(normalTexture, newCoords).b, texture(normalTexture, newCoords).g)), 0.0);
	//normalBuffer = vec4(((modelMat * vec4(normal2, 0.0)).xyz), 0.0);
	//normalBuffer = vec4(modelMat * vec4(normal2, 0.0));
	//normalBuffer = normalize(modelMat * vec4(texture(normalTexture, texCoord).rbg, 0.0));
	//normalBuffer = normalize(modelMat * vec4(normalMap, 0.0));
	//normalBuffer = vec4(h0 - h1) * 100.0;
	//normalBuffer = vec4(va, 0.0);
	//normalBuffer = vec4(TBN * normalize(normal2), 0.0);
	//normalBuffer = modelMat * vec4(normalize(vec3(texture(normalTexture, newCoords).r, texture(normalTexture, newCoords).g, texture(normalTexture, newCoords).b)), 0.0);
	normalBuffer = normalize(vec4(TBN2 * normalize((2.0 * (texture(normalTexture, newCoords).rgb) - 1.0)), 0.0));
	//normalBuffer = vec4(normalMatrix * normalize((2.0 * (texture(normalTexture, newCoords).rgb) - 1.0)), 0.0);
}
