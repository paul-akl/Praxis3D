//#version 150
//
//in vec2 position;
//
//void main()
//{
//    gl_Position = vec4(0.5, 0.5, 0.0, 1.0);
//}

#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

//in vec2 position;
//in vec3 color;

uniform mat4 MVP;

out vec3 color;
out vec2 texCoord;

void main() 
{

	//texCoord.x = (gl_VertexID == 2) ?  2.0 :  0.0;
	//texCoord.y = (gl_VertexID == 1) ?  2.0 :  0.0;

	//gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);

	//texCoord.x = (gl_VertexID == 2) ?  2.0 :  0.0;
	//texCoord.y = (gl_VertexID == 1) ?  2.0 :  0.0;

	//gl_Position = vec4(texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);

	color = vec3(vertexUV, 0.0);
	texCoord = vertexUV;
	vec3 pos = vertexPosition;
	//pos.z += 0.9;

	//pos /= vec3(2.0, 2.0, 2.0);

	//Color = vec3(vertexPosition.xy, 0.0);
	gl_Position = MVP * vec4(pos, 1.0);
	

  // Color = color;
   //gl_Position = vec4(position, 0.0, 1.0);
  // Color = vec3(position, 0.0);
  // gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
}