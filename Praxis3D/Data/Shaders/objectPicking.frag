#version 430 core

// Entity ID of the object being drawn, forwarded from the vertex shader
flat in uint passedEntityID;

// Picking pass renders into a single channel unsigned integer buffer
layout(location = 0) out uint outputEntityID;

void main()
{
	outputEntityID = passedEntityID;
}
