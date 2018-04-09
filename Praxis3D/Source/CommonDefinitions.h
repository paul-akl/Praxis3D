#pragma once

#include <GL\glew.h>

enum BindCommandType : unsigned int
{
	BindCommandType_Texture,
	BindCommandType_Framebuffer
};
enum CommandType : unsigned int
{
	CommandType_Draw,
	CommandType_ScreenSpaceDraw,
	CommandType_Bind,
	CommandType_Load
};

enum BufferType : unsigned int
{
	BufferType_Uniform		= GL_UNIFORM_BUFFER,
	BufferType_Array		= GL_ARRAY_BUFFER,
	BufferType_ElementArray = GL_ELEMENT_ARRAY_BUFFER
};
enum BufferUpdateType : unsigned int
{
	BufferUpdate_Data,		// updates the whole buffer at once
	BufferUpdate_SubData	// updates part of the buffer (based on offset and size)
};
enum BufferUsageHint : unsigned int
{
	BufferUsageHint_Stream  = GL_STREAM_DRAW,
	BufferUsageHint_Static  = GL_STATIC_DRAW,
	BufferUsageHint_Dynamic = GL_DYNAMIC_DRAW
};
enum LightBufferBinding : unsigned int
{
	LightBufferBinding_PointLight = 0,
	LightBufferBinding_SpotLight
};
enum LoadObjectType : unsigned int
{
	LoadObject_Buffer,
	LoadObject_Shader,
	LoadObject_Texture2D,
	LoadObject_TextureCube,
	LoadObject_Model
};
enum LoadableObjectType : unsigned int
{
	LoadableObj_Null,
	LoadableObj_ModelObj,
	LoadableObj_ShaderObj,
	LoadableObj_StaticEnvMap
};
enum MaterialType : unsigned int
{
	MaterialType_Diffuse,
	MaterialType_Normal,
	MaterialType_Emissive,
	MaterialType_Combined,
	MaterialType_NumOfTypes,
	MaterialType_Roughness = MaterialType_NumOfTypes,
	MaterialType_Metalness,
	MaterialType_Height,
	MaterialType_AmbientOcclusion,
	MaterialType_NumOfTypes_Extended
};
enum ModelBufferType : unsigned int
{
	// WARNING: do not modify, enum entries are order-sensitive
	// (Model::m_numElements depends on this order)

	ModelBuffer_Position = 0,
	ModelBuffer_Normal,
	ModelBuffer_TexCoord,
	ModelBuffer_Tangents,
	ModelBuffer_Bitangents,
	ModelBuffer_NumTypesWithoutIndex,
	ModelBuffer_Index = ModelBuffer_NumTypesWithoutIndex,
	ModelBuffer_NumAllTypes
};
enum ShaderType : unsigned int
{
	// WARNING: do not change the order - enum entries are order-sensitive

	ShaderType_Fragment,
	ShaderType_Geometry,
	ShaderType_Vertex,
	ShaderType_TessControl,
	ShaderType_TessEvaluation,
	ShaderType_NumOfTypes
};
enum TextureFormat : int
{
	TextureFormat_Red	= GL_RED,
	TextureFormat_Green = GL_GREEN,
	TextureFormat_Blue	= GL_BLUE,
	TextureFormat_Alpha = GL_ALPHA,
	TextureFormat_RGB	= GL_RGB,
	TextureFormat_RGBA	= GL_RGBA
};