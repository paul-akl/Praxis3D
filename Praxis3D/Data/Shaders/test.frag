//#version 150

//out vec4 outColor;

//void main()
//{
//   outColor = vec4(1.0, 1.0, 1.0, 1.0);
//}

#version 330 core

in vec2 texCoord;

out vec4 fragColor;

void main() 
{
   fragColor = vec4(0.0, 1.0, 0.0, 1.0);
}