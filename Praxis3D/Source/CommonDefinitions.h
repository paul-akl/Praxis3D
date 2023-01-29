#pragma once

#include <GL\glew.h>
#include <functional>

typedef std::uint32_t EntityID;
typedef unsigned int UpdateCount;
typedef std::vector<std::function<void()>> Functors;

constexpr EntityID NULL_ENTITY_ID = std::numeric_limits<EntityID>::max();

enum EngineStateType : unsigned int
{
	EngineStateType_MainMenu = 0,
	EngineStateType_Play
};

enum BindCommandType : unsigned int
{
	BindCommandType_Texture,
	BindCommandType_Framebuffer
};
enum CommandType : unsigned int
{
	CommandType_Draw,
	CommandType_ScreenSpaceDraw,
	CommandType_Compute,
	CommandType_Bind,
	CommandType_Load
};
enum MemoryBarrierType : unsigned int
{
	MemoryBarrierType_All,
	MemoryBarrierType_ImageAccessBarrier,
	MemoryBarrierType_AccessAndFetchBarrier,
	MemoryBarrierType_ShaderStorageBarrier
};
enum RenderPassType : unsigned int
{
	RenderPassType_Geometry,
	RenderPassType_Lighting,
	RenderPassType_AtmScattering,
	RenderPassType_HdrMapping,
	RenderPassType_Blur,
	RenderPassType_Bloom,
	RenderPassType_BloomComposite,
	RenderPassType_LenseFlare,
	RenderPassType_LenseFlareComposite,
	RenderPassType_Luminance,
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
enum BufferBindTarget : unsigned int
{
	BufferBindTarget_Uniform			= GL_UNIFORM_BUFFER,
	BufferBindTarget_AtomicCounter		= GL_ATOMIC_COUNTER_BUFFER,
	BufferBindTarget_TransformFeedback	= GL_TRANSFORM_FEEDBACK_BUFFER,
	BufferBindTarget_ShaderStorage		= GL_SHADER_STORAGE_BUFFER
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
enum LuminanceTextureType : unsigned int
{
	LensFlareTextureType_AverageLuminance = MaterialType_NumOfTypes_Extended
};
enum ShaderType : unsigned int
{
	// WARNING: do not change the order - enum entries are order-sensitive

	ShaderType_Compute,
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
enum TextureDataType : int
{
	TextureDataType_Float			= GL_FLOAT,
	TextureDataType_Int				= GL_INT,
	TextureDataType_UnsignedByte	= GL_UNSIGNED_BYTE
};
enum TextureDataFormat : int
{
	TextureDataFormat_R8		= GL_R8,
	TextureDataFormat_R16		= GL_R16,
	TextureDataFormat_R16F		= GL_R16F,
	TextureDataFormat_R32F		= GL_R32F,
	TextureDataFormat_RG8		= GL_RG8,
	TextureDataFormat_RG16		= GL_RG16,
	TextureDataFormat_RG16F		= GL_RG16F,
	TextureDataFormat_RG32F		= GL_RG32F,
	TextureDataFormat_RGB8		= GL_RGB8,
	TextureDataFormat_RGB16		= GL_RGB16,
	TextureDataFormat_RGB16F	= GL_RGB16F,
	TextureDataFormat_RGB32F	= GL_RGB32F,
	TextureDataFormat_RGBA8		= GL_RGBA8,
	TextureDataFormat_RGBA16	= GL_RGBA16,
	TextureDataFormat_RGBA16F	= GL_RGBA16F,
	TextureDataFormat_RGBA32F	= GL_RGBA32F,
	TextureDataFormat_R16I		= GL_R16I,
	TextureDataFormat_R32I		= GL_R32I,
	TextureDataFormat_R16UI		= GL_R16UI,
	TextureDataFormat_R32UI		= GL_R32UI
};
enum UniformBufferBinding : unsigned int
{
	UniformBufferBinding_PointLights,
	UniformBufferBinding_SpotLights,
	UniformBufferBinding_AtmScatParam,
	UniformBufferBinding_LensFlareParam
};