#pragma once

#include <GL\glew.h>
#include <functional>

#include "EnumFactory.h"
#include "Utilities.h"

typedef std::uint32_t EntityID;
typedef unsigned int UpdateCount;
typedef std::vector<std::function<void()>> Functors;

constexpr EntityID NULL_ENTITY_ID = std::numeric_limits<EntityID>::max();

#define COMPONENT_TYPE(Code) \
	/* Audio components */ \
	Code(ComponentType_SoundComponent, = 0) \
	Code(ComponentType_SoundListenerComponent, ) \
	/* Graphics components */ \
	Code(ComponentType_CameraComponent, ) \
	Code(ComponentType_LightComponent, ) \
	Code(ComponentType_ModelComponent, ) \
	Code(ComponentType_ShaderComponent, ) \
	/* GUI components */ \
	Code(ComponentType_GUISequenceComponent, ) \
	/* Physics components */ \
	Code(ComponentType_RigidBodyComponent, ) \
	/* Scripting components */ \
	Code(ComponentType_LuaComponent, ) \
	/* World components */ \
	Code(ComponentType_ObjectMaterialComponent, ) \
	Code(ComponentType_SpatialComponent, ) \
	Code(ComponentType_NumOfTypes, ) \
	Code(ComponentType_Entity, )
DECLARE_ENUM(ComponentType, COMPONENT_TYPE)

enum EngineChangeType : unsigned int
{
	EngineChangeType_None = 0,
	EngineChangeType_SceneFilename,
	EngineChangeType_SceneLoad,
	EngineChangeType_SceneReload,
	EngineChangeType_StateChange
};
enum EngineStateType : unsigned int
{
	EngineStateType_Default = 0,
	EngineStateType_MainMenu = EngineStateType_Default,
	EngineStateType_Play,
	EngineStateType_Editor,
	EngineStateType_NumOfTypes
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
#define RENDER_PASS_TYPE(Code) \
	Code(RenderPassType_Geometry, = 0) \
	Code(RenderPassType_Lighting,) \
	Code(RenderPassType_AtmScattering,) \
	Code(RenderPassType_HdrMapping,) \
	Code(RenderPassType_Blur,) \
	Code(RenderPassType_Bloom,) \
	Code(RenderPassType_BloomComposite,) \
	Code(RenderPassType_LenseFlare,) \
	Code(RenderPassType_LenseFlareComposite,) \
	Code(RenderPassType_Luminance,) \
	Code(RenderPassType_Final,) \
	Code(RenderPassType_GUI,) \
	Code(RenderPassType_AmbientOcclusion,) \
	Code(RenderPassType_NumOfTypes,)
DECLARE_ENUM(RenderPassType, RENDER_PASS_TYPE)
typedef std::vector<RenderPassType> RenderingPasses;

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
};
enum LoadObjectType : unsigned int
{
	LoadObject_Buffer,
	LoadObject_Shader,
	LoadObject_Texture2D,
	LoadObject_TextureCube,
	LoadObject_Model
};
enum UnloadObjectType : unsigned int
{
	UnloadObjectType_VAO,
	UnloadObjectType_Buffer,
	UnloadObjectType_Shader,
	UnloadObjectType_Texture,
	UnloadObjectType_NumOfTypes
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
	MaterialType_Noise,
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
	TextureDataType_Short			= GL_SHORT,
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
	TextureDataFormat_RGBA16SN	= GL_RGBA16_SNORM,
	TextureDataFormat_RGBA16F	= GL_RGBA16F,
	TextureDataFormat_RGBA32F	= GL_RGBA32F,
	TextureDataFormat_R16I		= GL_R16I,
	TextureDataFormat_R32I		= GL_R32I,
	TextureDataFormat_R16UI		= GL_R16UI,
	TextureDataFormat_R32UI		= GL_R32UI
};
enum TextureFilterType : int
{
	TextureFilterType_Linear				= GL_LINEAR,
	TextureFilterType_LinearMipmapLinear	= GL_LINEAR_MIPMAP_LINEAR,
	TextureFilterType_LinearMipmapNearest	= GL_LINEAR_MIPMAP_NEAREST,
	TextureFilterType_Nearest				= GL_NEAREST,
	TextureFilterType_NearestMipmapLinear	= GL_NEAREST_MIPMAP_LINEAR,
	TextureFilterType_NearestMipmapNearest	= GL_NEAREST_MIPMAP_NEAREST
};
enum TextureWrapType : int
{
	TextureWrapType_ClampToBorder		= GL_CLAMP_TO_BORDER,
	TextureWrapType_ClampToEdge			= GL_CLAMP_TO_EDGE,
	TextureWrapType_MirroredClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
	TextureWrapType_MirroredRepeat		= GL_MIRRORED_REPEAT,
	TextureWrapType_Repeat				= GL_REPEAT
};
enum UniformBufferBinding : unsigned int
{
	UniformBufferBinding_PointLights,
	UniformBufferBinding_SpotLights,
	UniformBufferBinding_AtmScatParam,
	UniformBufferBinding_LensFlareParam,
	UniformBufferBinding_AODataSet,
	UniformBufferBinding_SSAOSampleBuffer
};

enum GBufferTextureType : unsigned int
{
	GBufferPosition,
	GBufferDiffuse,
	GBufferNormal,
	GBufferEmissive,
	GBufferMatProperties,
	GBufferFinal,
	GBufferNumTextures = GBufferFinal,
	GBufferIntermediate,
	GBufferTotalNumTextures,
	GBufferInputTexture = GBufferTotalNumTextures,
	GbufferOutputTexture,
	GbufferDepth
};

enum AmbientOcclusionType : int
{
	AmbientOcclusionType_None = 0,
	AmbientOcclusionType_SSAO,
	AmbientOcclusionType_HBAO
};

enum AudioBusType : unsigned int
{
	AudioBusType_Ambient,
	AudioBusType_Master,
	AudioBusType_Music,
	AudioBusType_SFX,
	AudioBusType_NumOfTypes
};

#define OBJ_MATERIAL_ID(Code) \
	Code(Concrete, = 0) \
	Code(Glass,) \
	Code(Metal,) \
	Code(Plastic,) \
	Code(Rock,) \
	Code(Rubber,) \
	Code(Wood,) \
	Code(NumberOfMaterialTypes,) 
DECLARE_ENUM(ObjectMaterialType, OBJ_MATERIAL_ID)