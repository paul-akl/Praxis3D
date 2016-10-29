#version 330 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform float billboardScale;
uniform int depthType;
uniform mat4 Model;
uniform mat4 VP;

mat4 modelMat;

out vec2 texCoord;

void main()
{
	// Get the directions to which the geometry will be generated
	vec3 rightVec	= vec3(Model[0].x, Model[1].x, Model[2].x);
	vec3 upVec		= vec3(Model[0].y, Model[1].y, Model[2].y);

	modelMat[0].x = 1.0;		modelMat[1].y = 1.0;		modelMat[2].z = 1.0;	
	modelMat[3].x = Model[3].x; modelMat[3].y = Model[3].y; modelMat[3].z = Model[3].z;	modelMat[3].w = Model[3].w;

	// Get the point position
	vec3 position = gl_in[0].gl_Position.xyz;
	vec4 endPosition;

	// Offset the geometry, so it is generated from the center
	position += (rightVec * billboardScale / 2);
	position -= (upVec * billboardScale / 2);

	position -= (rightVec * billboardScale);
	endPosition = (VP * modelMat * vec4(position, 1.0)).xyzw;
	if(depthType == 1)
		gl_Position = endPosition.xyzw;
	if(depthType == 2)
		gl_Position = endPosition.xyww;
	texCoord = vec2(0.0, 0.0);
	EmitVertex();

	position += (upVec * billboardScale * 1.1);
	endPosition = (VP * modelMat * vec4(position, 1.0)).xyzw;
	if(depthType == 1)
		gl_Position = endPosition.xyzw;
	if(depthType == 2)
		gl_Position = endPosition.xyww;
	texCoord = vec2(0.0, 1.0);
	EmitVertex();
	
	position -= (upVec * billboardScale * 1.1);
	position += (rightVec * billboardScale);
	endPosition = (VP * modelMat * vec4(position, 1.0)).xyzw;
	if(depthType == 1)
		gl_Position = endPosition.xyzw;
	if(depthType == 2)
		gl_Position = endPosition.xyww;
	texCoord = vec2(1.0, 0.0);
	EmitVertex();
	
	position += (upVec * billboardScale * 1.1);
	endPosition = (VP * modelMat * vec4(position, 1.0)).xyzw;
	if(depthType == 1)
		gl_Position = endPosition.xyzw;
	if(depthType == 2)
		gl_Position = endPosition.xyww;
	texCoord = vec2(1.0, 1.0);
	EmitVertex();

	// 'Flush' the geometry
	EndPrimitive();
}