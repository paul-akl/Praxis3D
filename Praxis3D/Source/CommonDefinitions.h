#pragma once

#include <GL\glew.h>
#include <functional>

typedef std::uint32_t EntityID;
typedef unsigned int UpdateCount;
typedef std::vector<std::function<void()>> Functors;

constexpr EntityID NULL_ENTITY_ID = std::numeric_limits<EntityID>::max();

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
enum RenderPassType : unsigned int
{
	RenderPassType_Geometry,
	RenderPassType_Lighting,
	RenderPassType_AtmScattering,
	RenderPassType_HdrMapping,
	RenderPassType_Blur,
	RenderPassType_BloomComposite,
	RenderPassType_LenseFlare,
	RenderPassType_LenseFlareComposite,
	RenderPassType_Final,
	RenderPassType_GUI,
	RenderPassType_NumOfTypes
};

enum BufferType : unsigned int
{
	BufferType_Uniform			= GL_UNIFORM_BUFFER,
	BufferType_Array			= GL_ARRAY_BUFFER,
	BufferType_ElementArray		= GL_ELEMENT_ARRAY_BUFFER,
	BufferType_ShaderStorage	= GL_SHADER_STORAGE_BUFFER
};
enum BufferUpdateType : unsigned int
{
	BufferUpdate_Data,		// updates the whole buffer at once
	BufferUpdate_SubData	// updates part of the buffer (based on offset and size)
};
enum BufferUsageHint : unsigned int
{
	BufferUsageHint_StreamDraw		= GL_STREAM_DRAW,
	BufferUsageHint_StaticDraw		= GL_STATIC_DRAW,
	BufferUsageHint_DynamicDraw		= GL_DYNAMIC_DRAW,
	BufferUsageHint_DynamicCopy		= GL_DYNAMIC_COPY
};/*
enum LightBufferBinding : unsigned int
{
	LightBufferBinding_PointLight = 0,
	LightBufferBinding_SpotLight,
	LightBufferBinding_Total
};*/
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
enum AtmScatteringTextureType : unsigned int
{
	AtmScatteringTextureType_Irradiance = MaterialType_NumOfTypes_Extended,
	AtmScatteringTextureType_Scattering,
	AtmScatteringTextureType_SingleMie,
	AtmScatteringTextureType_Transmittance,
	AtmScatteringTextureType_NumOfTypes
};
enum LensFlareTextureType : unsigned int
{
	LensFlareTextureType_GhostGradient = MaterialType_NumOfTypes_Extended,
	LensFlareTextureType_LenseDirt,
	LensFlareTextureType_Starburst
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
enum SSBOBinding : unsigned int
{
	SSBOBinding_HDR = 0
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
enum UniformBufferBinding : unsigned int
{
	UniformBufferBinding_PointLights,
	UniformBufferBinding_SpotLights,
	UniformBufferBinding_AtmScatParam,
	UniformBufferBinding_LensFlareParam
};