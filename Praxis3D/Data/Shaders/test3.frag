//#version 150

//out vec4 outColor;

//void main()
//{
//   outColor = vec4(1.0, 1.0, 1.0, 1.0);
//}

#version 330 core

//in vec2 texCoord;
in vec3 color;
in vec2 texCoord;

out vec4 fragColor;

//layout(location = 0) out vec4 positionBuffer;
//layout(location = 1) out vec4 diffuseBuffer;
//layout(location = 2) out vec4 normalBuffer;
//layout(location = 3) out vec4 texCoordBuffer;
//layout(location = 4) out vec4 emissiveBuffer;

uniform sampler2D diffuseTexture;

void main() 
{
	//vec4 textureColor = texture(diffuseTexture, texCoord).rgba;
	//fragColor = vec4(textureColor.xyz, 1.0);
	fragColor = texture(diffuseTexture, texCoord).rgba;
	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}