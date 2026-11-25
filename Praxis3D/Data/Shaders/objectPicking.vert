#version 430 core

// Mesh buffers
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP;
uniform uint entityID;

// Entity ID of the object this mesh belongs to, forwarded to the fragment shader without
// interpolation, so every pixel of a mesh receives the exact same ID
flat out uint passedEntityID;

void main(void)
{
	gl_Position = MVP * vec4(vertexPosition, 1.0);
	passedEntityID = entityID;
}
