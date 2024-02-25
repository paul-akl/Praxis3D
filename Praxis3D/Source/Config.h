#pragma once

#include <climits>
#include <GL\glew.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "CommonDefinitions.h"
#include "ErrorCodes.h"
#include "EnumFactory.h"
#include "Utilities.h"

typedef uint64_t BitMask;

// Tests if the given bitmask contains the given flag; returns true if the flag bits are present in the bitmask
constexpr bool CheckBitmask(const BitMask p_bitmask, const BitMask p_flag) { return ((p_bitmask & p_flag) == p_flag); }

//#define ever ;;

//#define GL_LINEAR_MIPMAP_LINEAR 0x2703
//#define GL_LINEAR 0x2601
//#define GL_LESS 0x0201
//#define GL_BACK 0x0405

enum DataType : uint32_t
{
	DataType_Null = 0,
	// General
	DataType_CreateComponent,			// ComponentsConstructionInfo
	DataType_DeleteComponent,			// EntityAndComponent
	DataType_CreateEntity,				// ComponentsConstructionInfo
	DataType_DeleteEntity,				// EntityAndComponent
	// Graphics
	DataType_AtmScatteringData,			// AtmosphericScatteringData
	DataType_AmbientOcclusionData,		// AmbientOcclusionData
	DataType_GUIPassFunctors,			// FunctorSequence
	DataType_MiscSceneData,				// MiscSceneData
	DataType_RenderingPasses,			// RenderingPasses
	DataType_RenderToTexture,			// bool
	DataType_RenderToTextureResolution, // glm::ivec2
	DataType_ShadowMappingData,			// ShadowMappingData
	DataType_LoadShader,				// ShaderLoader::ShaderProgram
	DataType_LoadTexture2D,				// TextureLoader2D::Texture2DHandle
	DataType_UnloadTexture2D,			// unsigned int
	DataType_LoadTexture3D,
	DataType_UnloadTexture3D,
	DataType_UnloadModel,				// unsigned int
	DataType_ModelsProperties,			// ModelComponent::ModelsProperties
	// GUI
	DataType_AboutWindow,				// bool
	DataType_EnableGUISequence,			// bool
	DataType_EditorWindow,				// EditorWindowSettings
	DataType_FileBrowserDialog,			// FileBrowserDialog
	DataType_SettingsWindow,			// bool
	// Physics
	DataType_SimulationActive,			// bool
	// Scripting
	DataType_EnableLuaScripting,		// bool
	DataType_LuaVariables				// std::vector<std::pair<std::string, Property>>
};

namespace Systems
{
	#define TYPEID(Code) \
	Code(Null, = -1) \
	Code(Audio,) \
	Code(Graphics,) \
	Code(GUI,) \
	Code(Physics,) \
	Code(Script,) \
	Code(World,) \
	Code(NumberOfSystems,) 
	DECLARE_ENUM(TypeID, TYPEID)

	const static std::string SystemNames[NumberOfSystems] =
	{
		GetString(Audio),
		GetString(Graphics),
		GetString(GUI),
		GetString(Physics),
		GetString(Script),
		GetString(World)
	};
	
	namespace Types
	{
		static constexpr BitMask All = static_cast<BitMask>(-1);
		static constexpr BitMask Max = 32;// (BitMask)1 << ((CHAR_BIT * sizeof(BitMask) - 1));
	}
	namespace GameObjectComponents
	{
		static constexpr BitMask None		= (BitMask)1 << 0;
		static constexpr BitMask Graphics	= (BitMask)1 << 1;
		static constexpr BitMask GUI		= (BitMask)1 << 2;
		static constexpr BitMask Physics	= (BitMask)1 << 3;
		static constexpr BitMask Script		= (BitMask)1 << 4;
	}
	namespace GraphicsObjectComponents
	{
		static constexpr BitMask None		= (BitMask)1 << 0;
		static constexpr BitMask Camera		= (BitMask)1 << 1;
		static constexpr BitMask Lighting	= (BitMask)1 << 2;
		static constexpr BitMask Model		= (BitMask)1 << 3;
		static constexpr BitMask Shader		= (BitMask)1 << 4;
	}
	namespace GUIObjectComponents
	{
		static constexpr BitMask None		= (BitMask)1 << 0;
		static constexpr BitMask Sequence	= (BitMask)1 << 1;
	}
	namespace PhysicsObjectComponents
	{
		static constexpr BitMask None		= (BitMask)1 << 0;
		static constexpr BitMask RigidBody	= (BitMask)1 << 1;
		static constexpr BitMask SoftBody	= (BitMask)1 << 2;
	}
	namespace ScriptObjectComponents
	{
		static constexpr BitMask None		= (BitMask)1 << 0;
		static constexpr BitMask Lua		= (BitMask)1 << 1;
	}
	namespace AllComponentTypes
	{
		static constexpr BitMask None							= (BitMask)1 << 0;

		static constexpr BitMask AudioImpactSoundComponent		= (BitMask)1 << 1;
		static constexpr BitMask AudioSoundComponent			= (BitMask)1 << 2;
		static constexpr BitMask AudioSoundListenerComponent	= (BitMask)1 << 3;

		static constexpr BitMask GUISequenceComponent			= (BitMask)1 << 4;

		static constexpr BitMask GraphicsCameraComponent		= (BitMask)1 << 5;
		static constexpr BitMask GraphicsLightingComponent		= (BitMask)1 << 6;
		static constexpr BitMask GraphicsModelComponent			= (BitMask)1 << 7;
		static constexpr BitMask GraphicsShaderComponent		= (BitMask)1 << 8;

		static constexpr BitMask PhysicsCollisionEventComponent = (BitMask)1 << 9;
		static constexpr BitMask PhysicsCollisionShapeComponent = (BitMask)1 << 10;
		static constexpr BitMask PhysicsRigidBodyComponent		= (BitMask)1 << 11;

		static constexpr BitMask ScriptingLuaComponent			= (BitMask)1 << 12;

		static constexpr BitMask WorldMetadataComponent			= (BitMask)1 << 13;
		static constexpr BitMask WorldObjectMaterialComponent	= (BitMask)1 << 14;
		static constexpr BitMask WorldSpatialComponent			= (BitMask)1 << 15;
	}
	namespace Changes
	{
		namespace Common
		{
			static constexpr BitMask Shared1	= (BitMask)1 << 1;
			static constexpr BitMask Shared2	= (BitMask)1 << 2;
			static constexpr BitMask Shared3	= (BitMask)1 << 3;
			static constexpr BitMask Shared4	= (BitMask)1 << 4;
			static constexpr BitMask Shared5	= (BitMask)1 << 5;
			static constexpr BitMask Shared6	= (BitMask)1 << 6;
			static constexpr BitMask Shared7	= (BitMask)1 << 7;
			static constexpr BitMask Shared8	= (BitMask)1 << 8;
			static constexpr BitMask Shared9	= (BitMask)1 << 9;
			static constexpr BitMask Shared10	= (BitMask)1 << 10;
			static constexpr BitMask Shared11	= (BitMask)1 << 11;
			static constexpr BitMask Shared12	= (BitMask)1 << 12;
			static constexpr BitMask Shared13	= (BitMask)1 << 13;
			static constexpr BitMask Shared14	= (BitMask)1 << 14;
			static constexpr BitMask Shared15	= (BitMask)1 << 15;
			static constexpr BitMask Shared16	= (BitMask)1 << 16;
			static constexpr BitMask Shared17	= (BitMask)1 << 17;
			static constexpr BitMask Shared18	= (BitMask)1 << 18;
			static constexpr BitMask Shared19	= (BitMask)1 << 19;
			static constexpr BitMask Shared20	= (BitMask)1 << 20;
			static constexpr BitMask Shared21	= (BitMask)1 << 21;
			static constexpr BitMask Shared22	= (BitMask)1 << 22;
			static constexpr BitMask Shared23	= (BitMask)1 << 23;
			static constexpr BitMask Shared24	= (BitMask)1 << 24;
			static constexpr BitMask Shared25	= (BitMask)1 << 25;
			static constexpr BitMask Shared26	= (BitMask)1 << 26;
			static constexpr BitMask Shared27	= (BitMask)1 << 27;
			static constexpr BitMask Shared28	= (BitMask)1 << 28;
			static constexpr BitMask Shared29	= (BitMask)1 << 29;
			static constexpr BitMask Shared30	= (BitMask)1 << 30;
		}
		namespace Unique
		{
			static constexpr BitMask Unique1 = (BitMask)1 << 50;
			static constexpr BitMask Unique2 = (BitMask)1 << 51;
			static constexpr BitMask Unique3 = (BitMask)1 << 52;
			static constexpr BitMask Unique4 = (BitMask)1 << 53;
			static constexpr BitMask Unique5 = (BitMask)1 << 54;
		}
		namespace Type
		{
			static constexpr BitMask Generic	= (BitMask)1 << 63;
			static constexpr BitMask Spatial	= (BitMask)1 << 62;
			static constexpr BitMask Audio		= (BitMask)1 << 61;
			static constexpr BitMask Graphics	= (BitMask)1 << 60;
			static constexpr BitMask GUI		= (BitMask)1 << 59;
			static constexpr BitMask Physics	= (BitMask)1 << 58;
			static constexpr BitMask Script		= (BitMask)1 << 57;
			static constexpr BitMask World		= (BitMask)1 << 56;
		}

		namespace Generic
		{
			static constexpr BitMask CreateObject		= Changes::Type::Generic + Changes::Common::Shared1;
			static constexpr BitMask DeleteObject		= Changes::Type::Generic + Changes::Common::Shared2;
			static constexpr BitMask ExtendObject		= Changes::Type::Generic + Changes::Common::Shared3;
			static constexpr BitMask UnextendObject		= Changes::Type::Generic + Changes::Common::Shared4;
			static constexpr BitMask Link				= Changes::Type::Generic + Changes::Common::Shared5;

			static constexpr BitMask Active				= Changes::Type::Generic + Changes::Unique::Unique1;
			static constexpr BitMask Name				= Changes::Type::Generic + Changes::Unique::Unique2;

			static constexpr BitMask All				= CreateObject | DeleteObject | ExtendObject | Link | Active | Name;
		}
		namespace Spatial
		{
			static constexpr BitMask LocalPosition			= Changes::Type::Spatial + Changes::Common::Shared1;
			static constexpr BitMask LocalRotation			= Changes::Type::Spatial + Changes::Common::Shared2;
			static constexpr BitMask LocalScale				= Changes::Type::Spatial + Changes::Common::Shared3;
			static constexpr BitMask LocalTransform			= Changes::Type::Spatial + Changes::Common::Shared4;
			static constexpr BitMask LocalTransformNoScale	= Changes::Type::Spatial + Changes::Common::Shared5;

			static constexpr BitMask WorldPosition			= Changes::Type::Spatial + Changes::Common::Shared6;
			static constexpr BitMask WorldRotation			= Changes::Type::Spatial + Changes::Common::Shared7;
			static constexpr BitMask WorldScale				= Changes::Type::Spatial + Changes::Common::Shared8;
			static constexpr BitMask WorldTransform			= Changes::Type::Spatial + Changes::Common::Shared9;
			static constexpr BitMask WorldTransformNoScale	= Changes::Type::Spatial + Changes::Common::Shared10;

			static constexpr BitMask Velocity				= Changes::Type::Spatial + Changes::Common::Shared11;

			static constexpr BitMask AllLocalNoTransform	= LocalPosition | LocalRotation | LocalScale | Velocity;
			static constexpr BitMask AllWorldNoTransform	= WorldPosition | WorldRotation | WorldScale | Velocity;
			static constexpr BitMask AllLocal				= AllLocalNoTransform | LocalTransform | LocalTransformNoScale;
			static constexpr BitMask AllWorld				= AllWorldNoTransform | WorldTransform | WorldTransformNoScale;
			static constexpr BitMask All					= AllLocal | AllWorld;
		}
		namespace Audio
		{
			static constexpr BitMask SoundName				= Changes::Type::Audio + Changes::Common::Shared1;
			static constexpr BitMask ListenerID				= Changes::Type::Audio + Changes::Common::Shared2;
			static constexpr BitMask Loop					= Changes::Type::Audio + Changes::Common::Shared3;
			static constexpr BitMask Reload					= Changes::Type::Audio + Changes::Common::Shared4;
			static constexpr BitMask SoundType				= Changes::Type::Audio + Changes::Common::Shared5;
			static constexpr BitMask SoundSourceType		= Changes::Type::Audio + Changes::Common::Shared6;
			static constexpr BitMask Spatialized			= Changes::Type::Audio + Changes::Common::Shared7;
			static constexpr BitMask StartPlaying			= Changes::Type::Audio + Changes::Common::Shared8;
			static constexpr BitMask Volume					= Changes::Type::Audio + Changes::Common::Shared9;
			
			static constexpr BitMask VolumeAmbient			= Changes::Type::Audio + Changes::Common::Shared10;
			static constexpr BitMask VolumeMaster			= Changes::Type::Audio + Changes::Common::Shared11;
			static constexpr BitMask VolumeMusic			= Changes::Type::Audio + Changes::Common::Shared12;
			static constexpr BitMask VolumeSFX				= Changes::Type::Audio + Changes::Common::Shared13;
			static constexpr BitMask AllVolume				= VolumeAmbient | VolumeMaster | VolumeMusic | VolumeSFX;

			static constexpr BitMask All					= SoundName | ListenerID | Loop | Reload | SoundType | SoundSourceType | 
																Spatialized | StartPlaying | Volume | AllVolume;
		}
		namespace Graphics
		{
			static constexpr BitMask Lighting				= Changes::Type::Graphics + Changes::Common::Shared30;
			static constexpr BitMask Camera					= Changes::Type::Graphics + Changes::Common::Shared29;
			static constexpr BitMask Framebuffers			= Changes::Type::Graphics + Changes::Common::Shared28;
			static constexpr BitMask Scene					= Changes::Type::Graphics + Changes::Common::Shared27;

			static constexpr BitMask CameraID				= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared1;
			static constexpr BitMask Target					= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared2;
			static constexpr BitMask UpVector				= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared3;
			static constexpr BitMask FOV					= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared4;
			static constexpr BitMask ZFar					= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared5;
			static constexpr BitMask ZNear					= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared6;
			static constexpr BitMask AllCamera				= CameraID | Target | UpVector | FOV | ZFar | ZNear;

			static constexpr BitMask LightType				= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared7;
			static constexpr BitMask Color					= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared8;
			static constexpr BitMask CutoffAngle			= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared9;
			static constexpr BitMask Direction				= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared10;
			static constexpr BitMask Intensity				= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared11;
			static constexpr BitMask AllLighting			= LightType | Color | CutoffAngle | Direction | Intensity;

			static constexpr BitMask PositionBuffer			= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared12;
			static constexpr BitMask DiffuseBuffer			= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared13;
			static constexpr BitMask NormalBuffer			= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared14;
			static constexpr BitMask EmissiveBuffer			= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared15;
			static constexpr BitMask MatPropertiesBuffer	= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared16;
			static constexpr BitMask IntermediateBuffer		= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared17;
			static constexpr BitMask FinalBuffer			= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared18;
			static constexpr BitMask RenderToTextureBuffer	= Changes::Type::Graphics + Changes::Graphics::Framebuffers + Changes::Common::Shared19;
			static constexpr BitMask AllBuffers				= PositionBuffer | DiffuseBuffer | NormalBuffer | EmissiveBuffer | MatPropertiesBuffer | 
																IntermediateBuffer | FinalBuffer | RenderToTextureBuffer;

			static constexpr BitMask ActiveCameraID			= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared20;
			static constexpr BitMask AmbientIntensity		= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared21;
			static constexpr BitMask AOIntensity			= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared22;
			static constexpr BitMask AOBlurSharpness		= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared23;
			static constexpr BitMask AOBias					= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared24;
			static constexpr BitMask AONumOfSamples			= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared25;
			static constexpr BitMask AORadius				= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared26;
			static constexpr BitMask AOType					= Changes::Type::Graphics + Changes::Graphics::Scene + Changes::Common::Shared27;
			static constexpr BitMask AllScene				= ActiveCameraID | AmbientIntensity | AOIntensity | AOBlurSharpness | AOBias | AONumOfSamples | AORadius | AOType;

			static constexpr BitMask All					= AllCamera | AllLighting | AllBuffers | AllScene;
		}
		namespace GUI
		{
			static constexpr BitMask Sequence				= Changes::Type::GUI + Changes::Common::Shared1;
			static constexpr BitMask StaticSequence			= Changes::Type::GUI + Changes::Common::Shared2;
			static constexpr BitMask All					= Sequence | StaticSequence;
		}
		namespace Physics
		{
			static constexpr BitMask CollisionShapeSize		= Changes::Type::Physics + Changes::Common::Shared1;
			static constexpr BitMask CollisionShapeType		= Changes::Type::Physics + Changes::Common::Shared2;
			static constexpr BitMask Friction				= Changes::Type::Physics + Changes::Common::Shared3;
			static constexpr BitMask RollingFriction		= Changes::Type::Physics + Changes::Common::Shared4;
			static constexpr BitMask SpinningFriction		= Changes::Type::Physics + Changes::Common::Shared5;
			static constexpr BitMask Mass					= Changes::Type::Physics + Changes::Common::Shared6;
			static constexpr BitMask Restitution			= Changes::Type::Physics + Changes::Common::Shared7;
			static constexpr BitMask Kinematic				= Changes::Type::Physics + Changes::Common::Shared8;
			static constexpr BitMask Gravity				= Changes::Type::Physics + Changes::Common::Shared9;

			static constexpr BitMask All					= CollisionShapeType | CollisionShapeSize | Friction | RollingFriction | 
																SpinningFriction | Mass | Restitution | Kinematic | Gravity;
		}
		namespace Script
		{
			static constexpr BitMask Filename				= Changes::Type::Script + Changes::Common::Shared1;
			static constexpr BitMask PauseInEditor			= Changes::Type::Script + Changes::Common::Shared2;
			static constexpr BitMask Reload					= Changes::Type::Script + Changes::Common::Shared3;
			static constexpr BitMask All					= Filename | PauseInEditor | Reload;
		}
		namespace World
		{
			static constexpr BitMask ObjectMaterialType		= Changes::Type::World + Changes::Common::Shared1;
			static constexpr BitMask PrefabName				= Changes::Type::World + Changes::Common::Shared2;
			static constexpr BitMask All					= ObjectMaterialType | PrefabName;
		}

		static constexpr BitMask None = 0;
		static constexpr BitMask All = static_cast<BitMask>(-1);
	}
}

namespace Properties
{
	#define PROPERTYID(Code) \
	Code(Null, = 0) \
	/* General */ \
	Code(Active,) \
	Code(ArrayEntry,) \
	Code(ChangeController,) \
	Code(Components,) \
	Code(Default,) \
	Code(Enabled,) \
	Code(File,) \
	Code(Filename,) \
	Code(Index,) \
	Code(Keybindings,) \
	Code(LoadInBackground,) \
	Code(Major,) \
	Code(Minor,) \
	Code(Name,) \
	Code(None,) \
	Code(Objects,) \
	Code(ObjectPoolSize,) \
	Code(Patch,) \
	Code(Scene,) \
	Code(Source,) \
	Code(System,) \
	Code(Systems,) \
	Code(Type,) \
	Code(UniversalObject,) \
	Code(UniversalScene,) \
	Code(Value,) \
	Code(Variables,) \
	Code(Version,) \
	/* Audio */ \
	Code(Ambient,) \
	Code(Audio,) \
	Code(Banks,) \
	Code(Event,) \
	Code(Loop,) \
	Code(Master,) \
	Code(Music,) \
	Code(SoundComponent,) \
	Code(SoundEffect,) \
	Code(SoundListenerComponent,) \
	Code(Spatialized,) \
	Code(StartPlaying,) \
	Code(Volume,) \
	/* Geometry */ \
	Code(OffsetPosition,) \
	Code(OffsetRotation,) \
	Code(LocalPosition,) \
	Code(LocalRotation,) \
	Code(LocalRotationQuaternion,) \
	Code(LocalScale,) \
	Code(WorldPosition,) \
	Code(WorldRotation,) \
	Code(WorldRotationQuaternion,) \
	Code(WorldScale,) \
	/* Graphics */ \
	Code(AlphaThreshold, ) \
	Code(AmbientIntensity, ) \
	Code(AmbientOcclusion, ) \
	Code(Attenuation,) \
	Code(Back,) \
	Code(Bias,) \
	Code(BiasScale,) \
	Code(BiasMax,) \
	Code(BlurSamples,) \
	Code(BlurSharpness,) \
	Code(Bottom,) \
	Code(Camera,) \
	Code(CameraComponent,) \
	Code(CameraID,) \
	Code(Cascades,) \
	Code(ClampToBorder,) \
	Code(ClampToEdge,) \
	Code(Color,) \
	Code(CombinedTexture,) \
	Code(ComputeShader,) \
	Code(CutoffAngle,) \
	Code(Density,) \
	Code(Diffuse,) \
	Code(Direction,) \
	Code(Directions,) \
	Code(DirectionalLight,) \
	Code(Distance,) \
	Code(Divider,) \
	Code(Emissive,) \
	Code(EmissiveIntensity,) \
	Code(EnvironmentMapDynamic,) \
	Code(EnvironmentMapObject,) \
	Code(FaceCullingDraw,) \
	Code(FaceCullingShadow,) \
	Code(FOV,) \
	Code(FragmentShader,) \
	Code(Framing,) \
	Code(Front,) \
	Code(GeometryShader,) \
	Code(Graphics,) \
	Code(GraphicsObject,) \
	Code(Ground,) \
	Code(HBAO,) \
	Code(Height,) \
	Code(HeightScale,) \
	Code(Intensity,) \
	Code(Irradiance,) \
	Code(LightComponent,) \
	Code(Lighting,) \
	Code(Materials,) \
	Code(Metalness,) \
	Code(Meshes,) \
	Code(MirroredClampToEdge,) \
	Code(MirroredRepeat,) \
	Code(Models,) \
	Code(ModelComponent,) \
	Code(ModelObject,) \
	Code(ModelPoolSize,) \
	Code(Normal,) \
	Code(ParallaxHeightScale,) \
	Code(PenumbraScale,) \
	Code(PenumbraSize,) \
	Code(PenumbraScaleRange,) \
	Code(PCF,) \
	Code(PointLight,) \
	Code(PointLightPoolSize,) \
	Code(PostProcess,) \
	Code(Renderer,) \
	Code(Rendering,) \
	Code(RenderPasses,) \
	Code(Repeat,) \
	Code(Resolution,) \
	Code(RMHAO,) \
	Code(Roughness,) \
	Code(Samples,) \
	Code(Shaders,) \
	Code(ShaderComponent,) \
	Code(ShaderPoolSize,) \
	Code(ShaderGraphicsObject,) \
	Code(ShaderModelObject,) \
	Code(ShadowMapping,) \
	Code(SpotLight,) \
	Code(SpotLightPoolSize,) \
	Code(SSAO,) \
	Code(Static,) \
	Code(Steps,) \
	Code(StochasticSampling,) \
	Code(StochasticSamplingScale,) \
	Code(StochasticSamplingSeamFix,) \
	Code(Sun,) \
	Code(TessControlShader,) \
	Code(TessEvaluationShader,) \
	Code(TextureTilingFactor,) \
	Code(TextureScale,) \
	Code(TonemappingPass,) \
	Code(Top,) \
	Code(VertexShader,) \
	Code(WrapMode,) \
	Code(ZClipping,) \
	Code(ZFar,) \
	Code(ZNear,) \
	Code(ZPlaneMultiplier,) \
	/* Graphics atmospheric scattering*/ \
	Code(Absorption, ) \
	Code(Atmosphere, ) \
	Code(AtmosphericScattering, ) \
	Code(ConstantTerm,) \
	Code(ExpScale,) \
	Code(ExpTerm,) \
	Code(Extinction,) \
	Code(LinearTerm,) \
	Code(Mie,) \
	Code(Rayleigh,) \
	Code(Scattering,) \
	Code(Width,) \
	/* Graphics rendering passes */ \
	Code(AmbientOcclusionRenderPass,) \
	Code(AtmScatteringRenderPass,) \
	Code(BloomRenderPass,) \
	Code(FinalRenderPass,) \
	Code(GeometryRenderPass,) \
	Code(GUIRenderPass,) \
	Code(LightingRenderPass,) \
	Code(LuminanceRenderPass,) \
	Code(ShadowMappingPass,) \
	/* GUI */ \
	Code(EditorWindow,) \
	Code(GUI,) \
	Code(GUIObject,) \
	Code(GUISequenceComponent,) \
	Code(Sequence,) \
	/* Key binds */ \
	Code(BackwardKey,) \
	Code(CenterKey,) \
	Code(CloseKey,) \
	Code(DebugCaptureMouseKey,) \
	Code(DebugFullscreenKey,) \
	Code(DebugVertSyncKey,) \
	Code(DownKey,) \
	Code(ForwardKey,) \
	Code(LeftKey,) \
	Code(LeftStrafeKey,) \
	Code(ModifierKey,) \
	Code(NextKey,) \
	Code(PreviousKey,) \
	Code(RightKey,) \
	Code(RightStrafeKey,) \
	Code(SaveKey,) \
	Code(SprintKey,) \
	Code(UpKey,) \
	/* Linking */ \
	Code(ObjectLinks,) \
	Code(Observer,) \
	Code(Subject,) \
	/* Physics */ \
	Code(Box,) \
	Code(Capsule,) \
	Code(CollisionEventComponent,) \
	Code(CollisionShape,) \
	Code(CollisionShapeComponent,) \
	Code(Cone,) \
	Code(ConvexHull,) \
	Code(Cylinder,) \
	Code(Friction,) \
	Code(Gravity,) \
	Code(Kinematic,) \
	Code(Mass,) \
	Code(Physics,) \
	Code(PhysicsObject,) \
	Code(RollingFriction,) \
	Code(Restitution,) \
	Code(RigidBody,) \
	Code(RigidBodyComponent,) \
	Code(Size,) \
	Code(Sphere,) \
	Code(SpinningFriction,) \
	Code(Velocity,) \
	/* Script */ \
	Code(Angle,) \
	Code(Axis,) \
	Code(Azimuth,) \
	Code(BaseUIScript,) \
	Code(Day,) \
	Code(DayOfYear,) \
	Code(DebugMoveScript,) \
	Code(DebugRotateScript,) \
	Code(DebugUIScript,) \
	Code(FreeCamera,) \
	Code(Latitude,) \
	Code(Longitude,) \
	Code(LowerLimit,) \
	Code(Lua,) \
	Code(LuaComponent,) \
	Code(InputScript,) \
	Code(Hours,) \
	Code(KeyCode,) \
	Code(KeyName,) \
	Code(Minutes,) \
	Code(Month,) \
	Code(PauseInEditor,) \
	Code(Radius,) \
	Code(Script,) \
	Code(ScriptObject,) \
	Code(Seconds,) \
	Code(SolarTimeScript,) \
	Code(Speed,) \
	Code(SprintSpeed,) \
	Code(SunScript,) \
	Code(TimeMultiplier,) \
	Code(TimeZone,) \
	Code(Year,) \
	Code(UpperLimit,) \
	Code(WorldEditScript,) \
	Code(Zenith,) \
	/* Window */ \
	Code(Fullscreen,) \
	Code(MouseCapture,) \
	Code(VerticalSync,) \
	Code(WindowTitle,) \
	/* World */ \
	Code(Children,) \
	Code(Concrete,) \
	Code(GameObject,) \
	Code(Glass,) \
	Code(ID,) \
	Code(MetadataComponent,) \
	Code(Metal,) \
	Code(ObjectMaterialComponent,) \
	Code(Parent,) \
	Code(Plastic,) \
	Code(Prefab,) \
	Code(Rock,) \
	Code(Rubber,) \
	Code(SpatialComponent,) \
	Code(Wood,) \
	Code(World,) \
	/* End of property IDs */ \
	Code(NumberOfPropertyIDs,) 
	DECLARE_ENUM(PropertyID, PROPERTYID)

	// Declare a string array of all PropertyID names, that is used for matching strings to PropertyIDs
	DECLARE_NAME_ARRAY(PropertyID, PROPERTYID)

	// A few overloaded static functions to convert other values to PropertyID enum
	// Note: converting from string here is very slow, and would be better implemented
	// by using hash-maps or similar optimized string search algorithm

	static Properties::PropertyID toPropertyID(const int p_value)
	{
		// If the passed value is within enum range, static cast it to ID, if not, return null ID
		if(p_value > Properties::PropertyID::Null && p_value < Properties::PropertyID::NumberOfPropertyIDs)
			return static_cast<Properties::PropertyID>(p_value);
		else
			return Properties::PropertyID::Null;
	}
	static Properties::PropertyID toPropertyID(const float p_value)
	{
		// If the passed value is within enum range, static cast it to ID, if not, return null ID
		if((int)p_value > Properties::PropertyID::Null && (int)p_value < Properties::PropertyID::NumberOfPropertyIDs)
			return static_cast<Properties::PropertyID>((int)p_value);
		else
			return Properties::PropertyID::Null;
	}
	static Properties::PropertyID toPropertyID(const std::string &p_value)
	{
		// If string is empty return null ID
		if(p_value.empty() == true)
			return Properties::PropertyID::Null;

		// If the property ID is encoded in the string (in a format ID(propID), like so: "ID(12)" ),
		// extract the property ID from it, convert it to an int and return property ID from overloaded function
		if(p_value[0] == 'I' && p_value[1] == 'D' && p_value[2] == '(' && p_value[p_value.size() - 1] == ')')
			return toPropertyID(std::stoi(p_value.substr(3, p_value.size() - 1)));

		// Iterate over all PropertyIDs and compare the string to property name
		for(int i = 0; i < Properties::PropertyID::NumberOfPropertyIDs; i++)
			if(Properties::PropertyIDNames[i] == p_value)
				return static_cast<Properties::PropertyID>(i);

		// If this point is reached, no match was found, return null ID
		return Properties::PropertyID::Null;
	}
}

// Provides global read-only access to various configuration variables. A data-driven way of hard-coded values.
// To read values from a file, needs to be initialized and loaded before any system accesses the variables.
class Config
{
	// These friend classes are the only objects allowed to modify config variables:
	friend class AudioScene;
	friend class DebugUIScript;
	friend class DeferredRenderer;
	friend class EditorState;
	friend class EditorWindow;
	friend class Engine;
	friend class ErrorHandler;
	friend class LuaScript;
	friend class RendererFrontend;
	friend class Window;
public:
	struct AudioVariables
	{
		AudioVariables()
		{
			impact_impulse_param_divider = 1.0f;
			impact_impulse_volume_divider = 100.0f;
			impact_min_volume_threshold = 0.1f;
			impact_max_volume_threshold = 1.0f;
			impact_soft_hard_threshold = 30.0f;
			max_impact_volume = 2.0f;
			volume_ambient = 1.0f;
			volume_master = 0.5f;
			volume_music = 1.0f;
			volume_sfx = 1.0f;
			max_impact_audio_instances = 10;
			num_audio_channels = 32;
			bus_name_ambient = "Ambient";
			bus_name_master = "";
			bus_name_music = "Music";
			bus_name_prefix = "bus:/";
			bus_name_sfx = "SFX";
			channel_name_master = "Master";
			default_sound_bank = "Default\\Master.bank";
			default_sound_bank_string = "Default\\Master.strings.bank";
			default_impact_sound_bank = "Default\\Impact.bank";
			pathDelimiter = ":/";
		}

		float impact_impulse_param_divider;
		float impact_impulse_volume_divider;
		float impact_min_volume_threshold;
		float impact_max_volume_threshold;
		float impact_soft_hard_threshold;
		float max_impact_volume;
		float volume_ambient;
		float volume_master;
		float volume_music;
		float volume_sfx;
		int max_impact_audio_instances;
		int num_audio_channels;
		std::string bus_name_ambient;
		std::string bus_name_master;
		std::string bus_name_music;
		std::string bus_name_prefix;
		std::string bus_name_sfx;
		std::string channel_name_master;
		std::string default_sound_bank;
		std::string default_sound_bank_string;
		std::string default_impact_sound_bank;
		std::string pathDelimiter;
	};
	struct ComponentVariables
	{
		ComponentVariables()
		{
			camera_component_name = " (Camera Component)";
			component_name_separator = " - ";
			light_component_name = " (Light Component)";
			lua_component_name = " (Lua Component)";
			model_component_name = " (Model Component)";
			shader_component_name = " (Shader Component)";
		}

		std::string camera_component_name;
		std::string component_name_separator;
		std::string light_component_name;
		std::string lua_component_name;
		std::string model_component_name;
		std::string shader_component_name;
	};
	struct ConfigFileVariables
	{
		ConfigFileVariables()
		{
			config_file = "Data\\config.ini";
			error_code_strings_eng = "error-strings-eng.data";
		}

		std::string config_file;
		std::string error_code_strings_eng;
	};
	struct EngineVariables
	{
		EngineVariables()
		{
			change_ctrl_cml_notify_list_reserv = 4096;
			change_ctrl_grain_size = 50;
			change_ctrl_notify_list_reserv = 8192;
			change_ctrl_oneoff_data_list_reserv = 64;
			change_ctrl_oneoff_notify_list_reserv = 64;
			change_ctrl_subject_list_reserv = 8192;
			delta_time_divider = 1000;
			glsl_version = 430;
			gl_context_major_version = 3;
			gl_context_minor_version = 3;
			loaders_num_of_unload_per_frame = 1;
			log_max_num_of_logs = 200;
			object_directory_init_pool_size = 1000;
			smoothing_tick_samples = 100;
			task_scheduler_clock_frequency = 120;
			running = true;
			loadingState = true;
			log_store_logs = true;
			editorState = false;
			engineState = EngineStateType::EngineStateType_MainMenu;
		}

		int change_ctrl_cml_notify_list_reserv;
		int change_ctrl_grain_size;
		int change_ctrl_notify_list_reserv; 
		int change_ctrl_oneoff_data_list_reserv;
		int change_ctrl_oneoff_notify_list_reserv;
		int change_ctrl_subject_list_reserv;
		int delta_time_divider;
		int glsl_version;
		int gl_context_major_version;
		int gl_context_minor_version;
		int loaders_num_of_unload_per_frame;
		int log_max_num_of_logs;
		int object_directory_init_pool_size;
		int smoothing_tick_samples;
		int task_scheduler_clock_frequency;
		bool running;
		bool loadingState;
		bool log_store_logs;
		bool editorState;
		EngineStateType engineState;
	};
	struct FramebfrVariables
	{
		FramebfrVariables()
		{
			gl_position_buffer_internal_format = GL_RGB32F;
			gl_position_buffer_texture_format = GL_RGB;
			gl_position_buffer_texture_type = GL_FLOAT;

			gl_diffuse_buffer_internal_format = GL_RGBA16F;
			gl_diffuse_buffer_texture_format = GL_RGBA;
			gl_diffuse_buffer_texture_type = GL_FLOAT;

			gl_emissive_buffer_internal_format = GL_RGBA16F;
			gl_emissive_buffer_texture_format = GL_RGBA;
			gl_emissive_buffer_texture_type = GL_FLOAT;

			gl_normal_buffer_internal_format = GL_RGB16F;
			gl_normal_buffer_texture_format = GL_RGB;
			gl_normal_buffer_texture_type = GL_FLOAT;

			gl_mat_properties_buffer_internal_format = GL_RGBA16F;
			gl_mat_properties_buffer_texture_format = GL_RGBA;
			gl_mat_properties_buffer_texture_type = GL_FLOAT;

			gl_blur_buffer_internal_format = GL_RGBA16F;
			gl_blur_buffer_texture_format = GL_RGBA;
			gl_blur_buffer_texture_type = GL_FLOAT;

			gl_final_buffer_internal_format = GL_RGBA16F;
			gl_final_buffer_texture_format = GL_RGBA;
			gl_final_buffer_texture_type = GL_FLOAT;

			gl_depth_buffer_internal_format = GL_DEPTH_COMPONENT32F;
			gl_depth_buffer_texture_format = GL_DEPTH_COMPONENT;
			gl_depth_buffer_texture_type = GL_FLOAT;

			gl_buffers_min_filter = GL_LINEAR;
			gl_buffers_mag_filter = GL_LINEAR;
			gl_buffers_wrap_s_method = GL_CLAMP_TO_EDGE;
			gl_buffers_wrap_t_method = GL_CLAMP_TO_EDGE;

			gl_blur_buffer_min_filter = GL_LINEAR;
			gl_blur_buffer_mag_filter = GL_LINEAR;
			gl_blur_buffer_wrap_s_method = GL_CLAMP_TO_EDGE;
			gl_blur_buffer_wrap_t_method = GL_CLAMP_TO_EDGE;

			gl_final_buffer_min_filter_HDR = GL_LINEAR_MIPMAP_LINEAR;
			gl_final_buffer_min_filter = GL_LINEAR;
			gl_final_buffer_mag_filter = GL_LINEAR;
			gl_final_buffer_s_method = GL_CLAMP_TO_EDGE;
			gl_final_buffer_t_method = GL_CLAMP_TO_EDGE;
		}

		int gl_position_buffer_internal_format;
		int gl_position_buffer_texture_type;
		int gl_position_buffer_texture_format;

		int gl_diffuse_buffer_internal_format;
		int gl_diffuse_buffer_texture_format;
		int gl_diffuse_buffer_texture_type;

		int gl_emissive_buffer_internal_format;
		int gl_emissive_buffer_texture_format;
		int gl_emissive_buffer_texture_type;

		int gl_normal_buffer_internal_format;
		int gl_normal_buffer_texture_format;
		int gl_normal_buffer_texture_type;

		int gl_mat_properties_buffer_internal_format;
		int gl_mat_properties_buffer_texture_format;
		int gl_mat_properties_buffer_texture_type;

		int gl_blur_buffer_internal_format;
		int gl_blur_buffer_texture_type;
		int gl_blur_buffer_texture_format;

		int gl_final_buffer_internal_format;
		int gl_final_buffer_texture_type;
		int gl_final_buffer_texture_format;

		int gl_depth_buffer_internal_format;
		int gl_depth_buffer_texture_type;
		int gl_depth_buffer_texture_format;

		float gl_buffers_min_filter;
		float gl_buffers_mag_filter;
		int gl_buffers_wrap_s_method;
		int gl_buffers_wrap_t_method;

		float gl_blur_buffer_min_filter;
		float gl_blur_buffer_mag_filter;
		int gl_blur_buffer_wrap_s_method;
		int gl_blur_buffer_wrap_t_method;

		float gl_final_buffer_min_filter_HDR;
		float gl_final_buffer_min_filter;
		float gl_final_buffer_mag_filter;
		int gl_final_buffer_s_method;
		int gl_final_buffer_t_method;
	};
	struct GameplayVariables
	{
		GameplayVariables()
		{
			default_map = "default.pmap";
			main_menu_map = "mainMenu.pmap";
			play_map = "default.pmap";
			camera_freelook_speed = 10.0f;
		}

		std::string default_map;
		std::string main_menu_map;
		std::string play_map;
		float camera_freelook_speed;
	};
	struct GraphicsVariables
	{
		GraphicsVariables()
		{
			bloom_enabled = true;
			double_buffering = true;
			eye_adaption = false;
			multisampling = true;
			alpha_size = 8;
			antialiasing_type = 2;
			ao_blur_num_of_samples = 6;
			ao_num_of_directions = 8;
			ao_num_of_samples = 64;
			ao_num_of_steps = 4;
			ao_type = AmbientOcclusionType::AmbientOcclusionType_HBAO;
			bloom_blur_passes = 5;
			bloom_downscale_limit = 10;
			bloom_mipmap_limit = 16;
			current_resolution_x = 0;
			current_resolution_y = 0;
			dir_shadow_res_x = 2048;
			dir_shadow_res_y = 2048;
			lens_flare_blur_passes = 5;
			lens_flare_ghost_count = 4;
			multisample_buffers = 1;
			multisample_samples = 1;
			rendering_res_x = 1600;
			rendering_res_y = 900;
			render_to_texture_resolution_x = 1600;
			render_to_texture_resolution_y = 900;
			tonemap_method = 6;
			alpha_threshold = 0.0f;
			ambient_intensity_directional = 0.3f;
			ambient_intensity_point = 0.1f;
			ambient_intensity_spot = 0.1f;
			ao_hbao_bias = 0.1f;
			ao_ssao_bias = 0.025f;
			ao_blurSharpness = 40.0f;
			ao_hbao_intensity = 2.0f;
			ao_ssao_intensity = 2.0f;
			ao_hbao_radius = 2.0f;
			ao_ssao_radius = 0.5f;
			bloom_intensity = 1.0f;
			bloom_knee = 0.1f;
			bloom_threshold = 1.5f;
			bloom_dirt_intensity = 1.0f;
			emissive_multiplier = 0.0f;
			emissive_threshold = 0.01f;
			eye_adaption_rate = 0.25f;
			eye_adaption_intended_brightness = 0.5f;
			fog_color_x = 0.55f;
			fog_color_y = 0.55f;
			fog_color_z = 0.55f;
			fog_density = 0.003f;
			fov = 60.0f;
			gamma = 2.2f;
			lens_flare_aspect_ratio = 1.0f;
			lens_flare_chrom_abberration = 0.003f;
			lens_flare_downsample = 0.0f;
			lens_flare_ghost_spacing = 0.1f;
			lens_flare_ghost_threshold = 4.0f;
			lens_flare_halo_radius = 0.55f;
			lens_flare_halo_thickness = 0.2f;
			lens_flare_halo_threshold = 4.0f;
			light_atten_constant = 0.0f;
			light_atten_linear = 0.0f;
			light_atten_quadratic = 1.0f;
			light_color_r = 1.0f;
			light_color_g = 1.0f;
			light_color_b = 1.0f;
			LOD_parallax_mapping = 100.0f;
			luminance_multiplier = 0.5f;
			luminance_range_min = 0.004f;
			luminance_range_max = 11.3f;
			height_scale = 0.0f;
			stochastic_sampling_scale = 1.0f;
			texture_tiling_factor = 1.0f;
			z_far = 8000.0f;
			z_near = 0.1f;
		}

		bool bloom_enabled;
		bool double_buffering;
		bool eye_adaption;
		bool multisampling;
		int alpha_size;
		int antialiasing_type;
		int ao_blur_num_of_samples;
		int ao_num_of_directions;
		int ao_num_of_samples;
		int ao_num_of_steps;
		int ao_type;
		int bloom_blur_passes;
		int bloom_downscale_limit;
		int bloom_mipmap_limit;
		int current_resolution_x;
		int current_resolution_y;
		int dir_shadow_res_x;
		int dir_shadow_res_y;
		int lens_flare_blur_passes;
		int lens_flare_ghost_count;
		int multisample_buffers;
		int multisample_samples;
		int rendering_res_x;
		int rendering_res_y;
		int render_to_texture_resolution_x;
		int render_to_texture_resolution_y;
		int tonemap_method;
		float alpha_threshold;
		float ambient_intensity_directional;
		float ambient_intensity_point;
		float ambient_intensity_spot;
		float ao_hbao_bias;
		float ao_ssao_bias;
		float ao_blurSharpness;
		float ao_hbao_intensity;
		float ao_ssao_intensity;
		float ao_hbao_radius;
		float ao_ssao_radius;
		float bloom_intensity;
		float bloom_knee;
		float bloom_threshold;
		float bloom_dirt_intensity;
		float emissive_multiplier;
		float emissive_threshold;
		float eye_adaption_rate;
		float eye_adaption_intended_brightness;
		float fog_color_x;
		float fog_color_y;
		float fog_color_z;
		float fog_density;
		float fov;
		float gamma;
		float lens_flare_aspect_ratio;
		float lens_flare_chrom_abberration;
		float lens_flare_downsample;
		float lens_flare_ghost_spacing;
		float lens_flare_ghost_threshold;
		float lens_flare_halo_radius;
		float lens_flare_halo_thickness;
		float lens_flare_halo_threshold;
		float light_atten_constant;
		float light_atten_linear;
		float light_atten_quadratic;
		float light_color_r;
		float light_color_g;
		float light_color_b;
		float LOD_parallax_mapping;
		float luminance_multiplier;
		float luminance_range_min;
		float luminance_range_max;
		float height_scale;
		float stochastic_sampling_scale;
		float texture_tiling_factor;
		float z_far;
		float z_near;
	};
	struct GUIVariables
	{
		GUIVariables()
		{
			gui_docking_enabled = true;
			gui_render = true;
			gui_dark_style = true;
			gui_color_pallet = 1;
			gui_sequence_array_reserve_size = 50;
			about_window_font_size = 30.0f;
			editor_asset_selection_button_size_multiplier = 2.0f;
			editor_asset_texture_button_size_x = 60.0f;
			editor_asset_texture_button_size_y = 60.0f;
			editor_audio_banks_max_height = 100.0f;
			editor_float_slider_speed = 0.01f;
			editor_inspector_button_width_multiplier = 1.5f;
			editor_lua_variables_max_height = 200.0f;
			editor_play_button_size = 30.0f;
			editor_render_pass_max_height = 275.0f;
			gui_file_dialog_min_size_x = 400.0f;
			gui_file_dialog_min_size_y = 200.0f;
			gui_file_dialog_dir_color_R = 0.905f;
			gui_file_dialog_dir_color_G = 0.623f;
			gui_file_dialog_dir_color_B = 0.314f;
			loading_spinner_radius = 24.0f;
			loading_spinner_speed = 16.0f;
			loading_spinner_thickness = 3.0f; 
			about_window_font = "OpenSans-Regular.ttf";
			about_window_logo_texture = "logo2.png";
			editor_button_add_texture = "button_add_3.png";
			editor_button_add_list_texture = "button_add_from_list_1.png";
			editor_button_arrow_down_texture = "button_arrow_down_1.png";
			editor_button_arrow_up_texture = "button_arrow_up_1.png";
			editor_button_delete_entry_texture = "button_delete_5.png";
			editor_button_duplicate_entry_texture = "button_duplicate_1.png";
			editor_button_gui_sequence_texture = "button_gui_sequence_5.png";
			editor_button_guizmo_rotate_texture = "button_guizmo_rotate_1.png";
			editor_button_guizmo_translate_texture = "button_guizmo_translate_2.png";
			editor_button_open_file_texture = "button_open_file_1.png";
			editor_button_open_asset_list_texture = "button_add_from_list_1.png";
			editor_button_pause_texture = "button_editor_pause_2.png";
			editor_button_play_texture = "button_editor_play_2.png";
			editor_button_reload_texture = "button_reload_3.png";
			editor_button_restart_texture = "button_editor_restart_2.png";
			editor_button_scripting_enabled_texture = "button_scripting_3.png";
			editor_new_entity_name = "New Entity";
			gui_editor_window_name = "Editor window";
			url_github = "https://github.com/paul-akl/Praxis3D";
			url_pauldev = "http://www.pauldev.org/project-praxis3d.html";
		}
		bool gui_docking_enabled;
		bool gui_render;
		bool gui_dark_style;
		int gui_color_pallet;
		int gui_sequence_array_reserve_size;
		float about_window_font_size;
		float editor_asset_selection_button_size_multiplier;
		float editor_asset_texture_button_size_x;
		float editor_asset_texture_button_size_y;
		float editor_audio_banks_max_height;
		float editor_float_slider_speed;
		float editor_inspector_button_width_multiplier;
		float editor_lua_variables_max_height;
		float editor_play_button_size;
		float editor_render_pass_max_height;
		float gui_file_dialog_min_size_x;
		float gui_file_dialog_min_size_y;
		float gui_file_dialog_dir_color_R;
		float gui_file_dialog_dir_color_G;
		float gui_file_dialog_dir_color_B;
		float loading_spinner_radius;
		float loading_spinner_speed;
		float loading_spinner_thickness;
		std::string about_window_font;
		std::string about_window_logo_texture;
		std::string editor_button_add_texture;
		std::string editor_button_add_list_texture;
		std::string editor_button_arrow_down_texture;
		std::string editor_button_arrow_up_texture;
		std::string editor_button_delete_entry_texture;
		std::string editor_button_duplicate_entry_texture;
		std::string editor_button_gui_sequence_texture;
		std::string editor_button_guizmo_rotate_texture;
		std::string editor_button_guizmo_translate_texture;
		std::string editor_button_open_file_texture;
		std::string editor_button_open_asset_list_texture;
		std::string editor_button_pause_texture;
		std::string editor_button_play_texture;
		std::string editor_button_reload_texture;
		std::string editor_button_restart_texture;
		std::string editor_button_scripting_enabled_texture;
		std::string editor_new_entity_name;
		std::string gui_editor_window_name;
		std::string url_github;
		std::string url_pauldev;
	};
	struct InputVariables
	{
		InputVariables()
		{
			back_key = 42;
			backward_editor_key = 90;
			backward_key = 22;
			center_key = 93;
			clip_mouse_key = 66;
			close_window_key = 41;
			debug_1_key = 58;
			debug_2_key = 59;
			down_editor_key = 89;
			down_key = 6;
			escape_key = 41;
			forward_editor_key = 96;
			forward_key = 26;
			fullscreen_key = 68;
			jump_key = 44;
			left_editor_key = 92;
			left_strafe_key = 4;
			modifier_editor_key = 224;
			next_editor_key = 85;
			num_preallocated_keybinds = 110;
			previous_editor_key = 84;
			right_editor_key = 94;
			right_strafe_key = 7;
			save_editor_key = 22;
			sprint_key = 225;
			up_editor_key = 95;
			up_key = 44;
			vsync_key = 67;
			mouse_filter = false;
			mouse_warp_mode = false;
			mouse_jaw = 0.001f;
			mouse_pitch = 0.001f;
			mouse_pitch_clip = 1.2f;
			mouse_sensitivity = 1.0f;
		}

		int back_key;
		int backward_editor_key;
		int backward_key;
		int center_key;
		int clip_mouse_key;
		int close_window_key;
		int debug_1_key;
		int debug_2_key;
		int down_editor_key;
		int down_key;
		int escape_key;
		int forward_editor_key;
		int forward_key;
		int fullscreen_key;
		int jump_key;
		int left_editor_key;
		int left_strafe_key;
		int modifier_editor_key;
		int next_editor_key;
		int num_preallocated_keybinds;
		int previous_editor_key;
		int right_editor_key;
		int right_strafe_key;
		int save_editor_key;
		int sprint_key;
		int up_editor_key;
		int up_key;
		int vsync_key;
		bool mouse_filter;
		bool mouse_warp_mode;
		float mouse_jaw;
		float mouse_pitch;
		float mouse_pitch_clip;
		float mouse_sensitivity;
	};
	struct ModelVariables
	{
		ModelVariables()
		{
			calcTangentSpace = true;
			genBoundingBoxes = true;
			genNormals = false;
			genSmoothNormals = true;
			genUVCoords = true;
			joinIdenticalVertices = false;
			makeLeftHanded = false;
			optimizeCacheLocality = false;
			optimizeMeshes = true;
			optimizeGraph = false;
			removeComponent = false;
			triangulate = true;
		}

		bool calcTangentSpace;
		bool genBoundingBoxes;
		bool genNormals;
		bool genSmoothNormals;
		bool genUVCoords;
		bool joinIdenticalVertices;
		bool makeLeftHanded;
		bool optimizeCacheLocality;
		bool optimizeGraph;
		bool optimizeMeshes;
		bool removeComponent;
		bool triangulate;
	};
	struct ObjectPoolVariables
	{
		ObjectPoolVariables()
		{
			camera_component_default_pool_size = 5;
			light_component_default_pool_size = 100;
			lua_component_default_pool_size = 5;
			gui_sequence_component_default_pool_size = 5;
			model_component_default_pool_size = 100;
			object_pool_size = 50;
			point_light_pool_size = 50;
			regid_body_component_default_pool_size = 100;
			shader_component_default_pool_size = 5;
			spatial_component_default_pool_size = 100;
			spot_light_pool_size = 25;
			sound_component_default_pool_size = 50;
			sound_listener_component_default_pool_size = 50;
		}

		int camera_component_default_pool_size;
		int light_component_default_pool_size;
		int lua_component_default_pool_size;
		int gui_sequence_component_default_pool_size;
		int model_component_default_pool_size;
		int object_pool_size;
		int point_light_pool_size;
		int regid_body_component_default_pool_size;
		int shader_component_default_pool_size;
		int spatial_component_default_pool_size;
		int spot_light_pool_size;
		int sound_component_default_pool_size;
		int sound_listener_component_default_pool_size;
	};
	struct PathsVariables
	{
		PathsVariables()
		{
			config_path = "Data\\";
			engine_assets_path = "Default\\";
			font_path = "Data\\Fonts\\";
			gui_assets_path = "Default\\GUI\\";
			map_path = "Data\\Maps\\";
			model_path = "Data\\Models\\";
			object_path = "Data\\Objects\\";
			prefab_path = "Data\\Prefabs\\";
			script_path = "Data\\Scripts\\";
			shader_path = "Data\\Shaders\\";
			sound_path = "Data\\Sounds\\";
			texture_path = "Data\\Materials\\";
		}

		std::string config_path;
		std::string engine_assets_path;
		std::string font_path;
		std::string gui_assets_path;
		std::string map_path;
		std::string model_path;
		std::string object_path;
		std::string prefab_path;
		std::string script_path;
		std::string shader_path;
		std::string sound_path;
		std::string texture_path;
	};
	struct PhysicsVariables
	{
		PhysicsVariables()
		{
			applied_impulse_threshold = 1.0f;
			life_time_threshold = 2;
		}
		float applied_impulse_threshold;
		int life_time_threshold;
	};
	struct RendererVariables
	{
		RendererVariables()
		{
			atm_scattering_ground_vert_shader = "atmosphericScatteringPass_ground.vert";
			atm_scattering_ground_frag_shader = "atmosphericScatteringPass_ground.frag";
			atm_scattering_sky_vert_shader = "atmosphericScatteringPass_sky.vert";
			atm_scattering_sky_frag_shader = "atmosphericScatteringPass_sky.frag";
			dir_light_vert_shader = "dirLightPass.vert";
			dir_light_frag_shader = "dirLightPass.frag";
			point_light_vert_shader = "pointLightPass.vert";
			point_light_frag_shader = "pointLightPass.frag";
			spot_light_vert_shader = "spotLightPass.vert";
			spot_light_frag_shader = "spotLightPass.frag";
			dir_light_quad = "quad.obj";
			point_light_sphere = "sphere.obj";
			spot_light_cone = "cone.3ds";
			stencil_pass_vert_shader = "stencilPass.vert";
			stencil_pass_frag_shader = "stencilPass.frag";
			exposure_adaptation_frag_shader = "exposureAdaptation.frag";
			exposure_adaptation_vert_shader = "exposureAdaptation.vert";
			geometry_pass_vert_shader = "geometryPass.vert";
			geometry_pass_frag_shader = "geometryPass.frag";
			geom_billboard_vert_shader = "geomBillboard.vert";
			geom_billboard_frag_shader = "geomBillboard.frag";
			geom_billboard_goem_shader = "geomBillboard.geom";
			gaussian_blur_vertical_frag_shader = "gaussianBlurVertical.frag";
			gaussian_blur_vertical_vert_shader = "gaussianBlurVertical.vert";
			gaussian_blur_horizontal_frag_shader = "gaussianBlurHorizontal.frag";
			gaussian_blur_horizontal_vert_shader = "gaussianBlurHorizontal.vert";
			hdr_mapping_pass_frag_shader = "hdrMappingPass.frag";
			hdr_mapping_pass_vert_shader = "hdrMappingPass.vert";
			luminance_average_comp_shader = "luminanceAverage.comp";
			luminance_histogram_comp_shader = "luminanceHistogram.comp";
			tonemapping_vert_shader = "tonemapping.vert";
			tonemapping_frag_shader = "tonemapping.frag";
			bloom_composite_pass_vert_shader = "bloomCompositePass.vert";
			bloom_composite_pass_frag_shader = "bloomCompositePass.frag";
			bloom_downscale_comp_shader = "bloomDownscale.comp";
			bloom_upscale_comp_shader = "bloomUpscale.comp";
			blur_pass_vert_shader = "blurPass.vert";
			blur_pass_frag_shader = "blurPass.frag";
			csm_pass_layered_frag_shader = "csmPassLayered.frag";
			csm_pass_layered_geom_shader = "csmPassLayered.geom";
			csm_pass_layered_vert_shader = "csmPassLayered.vert";
			csm_pass_single_frag_shader = "csmPassSingle.frag";
			csm_pass_single_vert_shader = "csmPassSingle.vert";
			hbao_blur_horizontal_frag_shader = "ambientOcclusionBlurHBAOhorizontal.frag";
			hbao_blur_horizontal_vert_shader = "ambientOcclusionBlurHBAO.vert";
			hbao_blur_vertical_frag_shader = "ambientOcclusionBlurHBAOvertical.frag";
			hbao_blur_vertical_vert_shader = "ambientOcclusionBlurHBAO.vert";
			hbao_pass_frag_shader = "ambientOcclusionPassHBAO.frag";
			hbao_pass_vert_shader = "ambientOcclusionPassHBAO.vert";
			lense_flare_comp_pass_vert_shader = "lenseFlareCompositePass.vert";
			lense_flare_comp_pass_frag_shader = "lenseFlareCompositePass.frag";
			lense_flare_pass_vert_shader = "lenseFlarePass.vert";
			lense_flare_pass_frag_shader = "lenseFlarePass.frag";
			light_pass_vert_shader = "lightPass.vert";
			light_pass_frag_shader = "lightPass.frag";
			final_pass_vert_shader = "finalPass.vert";
			final_pass_frag_shader = "finalPass.frag";
			postProcess_pass_vert_shader = "postProcessPass.vert";
			postProcess_pass_frag_shader = "postProcessPass.frag";
			reflection_pass_vert_shader = "reflectionPass.vert";
			reflection_pass_frag_shader = "reflectionPass.frag";
			lens_flare_dirt_texture = "lensDirtMask.png";
			lens_flare_ghost_gradient_texture = "p3d_lensFlareGhostColorGradient.png";
			lens_flare_starburst_texture = "p3d_lensFlareStarburst.png";
			ssao_blur_frag_shader = "ambientOcclusionBlurSSAO.frag";
			ssao_blur_vert_shader = "ambientOcclusionBlurSSAO.vert";
			ssao_pass_frag_shader = "ambientOcclusionPassSSAO.frag";
			ssao_pass_vert_shader = "ambientOcclusionPassSSAO.vert";
			texture_repetition_noise_texture = "gray_noise_medium.png";
			csm_bias_scale = 0.0004f;
			csm_max_shadow_bias = 0.0005f;
			csm_penumbra_size = 1.42f;
			csm_penumbra_size_scale_min = 1.0f;
			csm_penumbra_size_scale_max = 2000.0f;
			current_viewport_position_x = 0.0f;
			current_viewport_position_y = 0.0f;
			dir_light_quad_offset_x = 0.0f;
			dir_light_quad_offset_y = 0.0f;
			dir_light_quad_offset_z = 0.0f;
			dir_light_quad_rotation_x = 180.0f;
			dir_light_quad_rotation_y = 0.0f;
			dir_light_quad_rotation_z = 0.0f;
			fxaa_edge_threshold_min = 0.0312f;
			fxaa_edge_threshold_max = 0.125f;
			fxaa_edge_subpixel_quality = 0.75f;
			parallax_mapping_min_steps = 8.0f;
			parallax_mapping_max_steps = 32.0f;
			csm_num_of_pcf_samples = 16;
			csm_resolution = 2048;
			current_viewport_size_x = 0;
			current_viewport_size_y = 0;
			depth_test_func = GL_LESS;
			face_culling_mode = GL_BACK;
			fxaa_iterations = 12;
			heightmap_combine_channel = 3;
			heightmap_combine_texture = 1;
			max_num_point_lights = 450;
			max_num_spot_lights = 50;
			objects_loaded_per_frame = 1;
			parallax_mapping_method = 5;
			render_to_texture_buffer = GBufferTextureType::GBufferEmissive;
			shader_pool_size = 10;
			ssao_num_of_samples = 64;
			csm_face_culling = true;
			csm_front_face_culling = true;
			depth_test = true;
			face_culling = true;
			fxaa_enabled = true;
			msaa_enabled = false;
			stochastic_sampling_seam_fix = true;
		}
		
		std::string atm_scattering_ground_vert_shader;
		std::string atm_scattering_ground_frag_shader;
		std::string atm_scattering_sky_vert_shader;
		std::string atm_scattering_sky_frag_shader;
		std::string dir_light_vert_shader;
		std::string dir_light_frag_shader;
		std::string point_light_vert_shader;
		std::string point_light_frag_shader;
		std::string spot_light_vert_shader;
		std::string spot_light_frag_shader;
		std::string dir_light_quad;
		std::string point_light_sphere;
		std::string spot_light_cone;
		std::string stencil_pass_vert_shader;
		std::string stencil_pass_frag_shader;
		std::string exposure_adaptation_frag_shader;
		std::string exposure_adaptation_vert_shader;
		std::string geometry_pass_vert_shader;
		std::string geometry_pass_frag_shader;
		std::string geom_billboard_vert_shader;
		std::string geom_billboard_frag_shader;
		std::string geom_billboard_goem_shader;
		std::string gaussian_blur_vertical_frag_shader;
		std::string gaussian_blur_vertical_vert_shader;
		std::string gaussian_blur_horizontal_frag_shader;
		std::string gaussian_blur_horizontal_vert_shader;
		std::string hdr_mapping_pass_frag_shader;
		std::string hdr_mapping_pass_vert_shader;
		std::string luminance_average_comp_shader;
		std::string luminance_histogram_comp_shader;
		std::string tonemapping_vert_shader;
		std::string tonemapping_frag_shader;
		std::string bloom_composite_pass_vert_shader;
		std::string bloom_composite_pass_frag_shader;
		std::string bloom_downscale_comp_shader;
		std::string bloom_upscale_comp_shader;
		std::string blur_pass_vert_shader;
		std::string blur_pass_frag_shader;
		std::string csm_pass_layered_frag_shader;
		std::string csm_pass_layered_geom_shader;
		std::string csm_pass_layered_vert_shader;
		std::string csm_pass_single_frag_shader;
		std::string csm_pass_single_vert_shader;
		std::string hbao_blur_horizontal_frag_shader;
		std::string hbao_blur_horizontal_vert_shader;
		std::string hbao_blur_vertical_frag_shader;
		std::string hbao_blur_vertical_vert_shader;
		std::string hbao_pass_frag_shader;
		std::string hbao_pass_vert_shader;
		std::string lense_flare_comp_pass_vert_shader;
		std::string lense_flare_comp_pass_frag_shader;
		std::string lense_flare_pass_vert_shader;
		std::string lense_flare_pass_frag_shader;
		std::string light_pass_vert_shader;
		std::string light_pass_frag_shader;
		std::string final_pass_vert_shader;
		std::string final_pass_frag_shader;
		std::string postProcess_pass_vert_shader;
		std::string postProcess_pass_frag_shader;
		std::string reflection_pass_vert_shader;
		std::string reflection_pass_frag_shader;
		std::string lens_flare_dirt_texture;
		std::string lens_flare_ghost_gradient_texture;
		std::string lens_flare_starburst_texture;
		std::string ssao_blur_frag_shader;
		std::string ssao_blur_vert_shader;
		std::string ssao_pass_frag_shader;
		std::string ssao_pass_vert_shader;
		std::string texture_repetition_noise_texture;
		float csm_bias_scale;
		float csm_max_shadow_bias;
		float csm_penumbra_size;
		float csm_penumbra_size_scale_min;
		float csm_penumbra_size_scale_max;
		float current_viewport_position_x;
		float current_viewport_position_y;
		float dir_light_quad_offset_x;
		float dir_light_quad_offset_y;
		float dir_light_quad_offset_z;
		float dir_light_quad_rotation_x;
		float dir_light_quad_rotation_y;
		float dir_light_quad_rotation_z;
		float fxaa_edge_threshold_min;
		float fxaa_edge_threshold_max;
		float fxaa_edge_subpixel_quality;
		float parallax_mapping_min_steps;
		float parallax_mapping_max_steps;
		int csm_num_of_pcf_samples;
		int csm_resolution;
		int current_viewport_size_x;
		int current_viewport_size_y;
		int depth_test_func;
		int face_culling_mode;
		int fxaa_iterations;
		int heightmap_combine_channel;
		int heightmap_combine_texture;
		int max_num_point_lights;
		int max_num_spot_lights;
		int objects_loaded_per_frame;
		int parallax_mapping_method;
		int render_to_texture_buffer;
		int shader_pool_size;
		int ssao_num_of_samples;
		bool csm_face_culling;
		bool csm_front_face_culling;
		bool depth_test;
		bool face_culling;
		bool fxaa_enabled;
		bool msaa_enabled;
		bool stochastic_sampling_seam_fix;
	};
	struct ScriptVariables
	{
		ScriptVariables()
		{
			defaultScriptFilename = "Default_script.lua";
			iniFunctionName = "init";
			updateFunctionName = "update";
			createObjectFunctionName = "create";
			userTypeTableName = "Types";
			luaUpdateErrorsEveryFrame = true;
		}

		std::string defaultScriptFilename;
		std::string iniFunctionName;
		std::string updateFunctionName;
		std::string createObjectFunctionName;
		std::string userTypeTableName;
		bool luaUpdateErrorsEveryFrame;
	};
	struct ShaderVariables
	{
		ShaderVariables()
		{
			atmScatProjMatUniform = "atmScatProjMat";
			modelMatUniform = "modelMat";
			viewMatUniform = "viewMat";
			projectionMatUniform = "projMat";
			viewProjectionMatUniform = "viewProjMat";
			modelViewMatUniform = "modelViewMat";
			modelViewProjectionMatUniform = "MVP";
			transposeViewMatUniform = "transposeViewMat";
			transposeInverseViewMatUniform = "transposeInverseViewMat";
			screenSizeUniform = "screenSize";
			inverseScreenSizeUniform = "inverseScreenSize";
			screenNumOfPixelsUniform = "screenNumOfPixels";
			deltaTimeMSUniform = "deltaTimeMS";
			deltaTimeSUniform = "deltaTimeS";
			elapsedTimeUniform = "elapsedTime";
			gammaUniform = "gamma";
			alphaCullingUniform = "alphaCulling";
			alphaThresholdUniform = "alphaThreshold";
			emissiveMultiplierUniform = "emissiveMultiplier";
			emissiveThresholdUniform = "emissiveThreshold";
			heightScaleUniform = "heightScale"; 
			parallaxMappingNumOfStepsLayersUniform = "pomNumOfSteps";
			textureTilingFactorUniform = "textureTilingFactor";
			stochasticSamplingScaleUniform = "stochasticSamplingScale";
			LODParallaxUniform = "parallaxMappingLOD";
			texelSize = "texelSize";
			numOfTexels = "numOfTexels";
			mipLevel = "mipLevel";
			projPlaneRange = "projPlaneRange";

			ambientLightIntensity = "ambientLightIntensity";
			dirLightColor = "directionalLight.m_color";
			dirLightDirection = "directionalLight.m_direction";
			dirLightIntensity = "directionalLight.m_intensity";
			numPointLightsUniform = "numPointLights";
			numSpotLightsUniform = "numSpotLights";
			pointLightViewProjectionMatUniform = "pointLightMVP";
			pointLightBuffer = "PointLights";
			spotLightBuffer = "SpotLights";
			spotLightViewProjectionMatUniform = "spotLightMVP";
			stencilPassViewProjectionMatUniform = "stencilMVP";

			dirShadowMapMVPUniform = "dirShadowMapMVP";
			dirShadowMapBiasMVPUniform = "dirShadowMapBiasMVP";

			cameraPosVecUniform = "cameraPosVec";
			cameraTargetVecUniform = "cameraTargetVec";
			cameraUpVecUniform = "cameraUpVec";
			cameraRightVecUniform = "cameraRightVec";

			positionMapUniform = "positionMap";
			diffuseMapUniform = "diffuseMap";
			normalMapUniform = "normalMap";
			emissiveMapUniform = "emissiveMap";
			matPropertiesMapUniform = "matPropertiesMap";
			intermediateMapUniform = "intermediateMap";
			finalMapUniform = "finalColorMap";
			depthMapUniform = "depthMap";
			inputColorMapUniform = "inputColorMap";
			outputColorMapUniform = "outputColorMap";

			csmBiasScaleUniform = "csmBiasScale";
			csmDepthMapUniform = "csmDepthMap";
			csmPenumbraScaleRange = "csmPenumbraScaleRange";

			sunGlowTextureUniform = "sunGlowMap";
			skyMapTextureUniform = "skyMap";
			dirShadowMapTextureUniform = "dirShadowMap";
			diffuseTextureUniform = "diffuseTexture";
			normalTextureUniform = "normalTexture";
			specularTextureUniform = "specularTexture";
			emissiveTextureUniform = "emissiveTexture";
			glossTextureUniform = "glossTexture";
			heightTextureUniform = "heightTexture";
			combinedTextureUniform = "combinedTexture";
			averageLuminanceTexture = "averageLuminanceTexture";
			noiseTexture = "noiseTexture";

			hbaoBlurHorizontalInvResDirection = "hbaoBlurHorizontalInvResDirection";
			hbaoBlurVerticalInvResDirection = "hbaoBlurVerticalInvResDirection";
			hbaoBlurNumOfSamples = "hbaoBlurNumOfSamples";
			hbaoBlurSharpness = "hbaoBlurSharpness";

			atmIrradianceTextureUniform = "atmIrradianceTexture";
			atmScatteringTextureUniform = "atmScatteringTexture";
			atmSingleMieScatTextureUniform = "atmSingleMieTexture";
			atmTransmittanceTextureUniform = "atmTransmitTexture";

			bloomTreshold = "bloomTreshold";
			bloomIntensity = "bloomIntensity";
			bloomDirtIntensity = "bloomDirtIntensity";

			inverseLogLuminanceRange = "inverseLogLuminanceRange";
			logLuminanceRange = "logLuminanceRange";
			luminanceMultiplier = "luminanceMultiplier";
			minLogLuminance = "minLogLuminance";
			tonemapMethod = "tonemapMethod";

			lensFlareDirtTextureUniform = "lensDirtTexture";
			lensFlareGhostGradientTextureUniform = "ghostGradientTexture";
			lensFlareStarburstTextureUniform = "lenseStarburstTexture";
			
			dynamicEnvMapUniform = "dynamicEnvMap";
			staticEnvMapUniform = "staticEnvMap";

			fogDensityUniform = "fogDensity";
			fogColorUniform = "fogColor";
			billboardScaleUniform = "billboardScale";
			depthTypeUniform = "depthType";

			AODataSetBuffer = "AODataSetBuffer";
			atmScatParamBuffer = "AtmScatParametersBuffer";
			CSMDataSetBuffer = "CSMDataSetBuffer";
			eyeAdaptionRateUniform = "eyeAdaptionRate";
			eyeAdaptionIntBrightnessUniform = "eyeAdaptionIntBrightness";
			HDRSSBuffer = "HDRBuffer";
			lensFlareParametersBuffer = "LensFlareParametersBuffer";
			materialDataBuffer = "MaterialDataBuffer";
			SSAOSampleBuffer = "SSAOSampleBuffer";

			testMatUniform = "testMat";
			testVecUniform = "testVec";
			testFloatUniform = "testFloat";

			define_alpha_discard = "ALPHA_DISCARD";
			define_fxaa = "FXAA";
			define_fxaa_edge_threshold_min = "FXAA_EDGE_THRESHOLD_MIN";
			define_fxaa_edge_threshold_max = "FXAA_EDGE_THRESHOLD_MAX";
			define_fxaa_iterations = "FXAA_ITERATIONS";
			define_fxaa_subpixel_quality = "FXAA_SUBPIXEL_QUALITY";
			define_maxNumOfPointLights = "MAX_NUM_POINT_LIGHTS";
			define_maxNumOfSpotLights = "MAX_NUM_SPOT_LIGHTS";
			define_normalMapCompression = "NORMAL_MAP_COMPRESSION";
			define_numOfCascades = "NUM_OF_CASCADES";
			define_numOfMaterialTypes = "NUM_OF_MATERIAL_TYPES";
			define_numOfPCFSamples = "NUM_OF_PCF_SAMPLES";
			define_parallaxMapping = "PARALLAX_MAPPING";
			define_parallaxMappingMethod = "PARALLAX_MAPPING_METHOD";
			define_shadowMapping = "SHADOW_MAPPING";
			define_stochasticSampling = "STOCHASTIC_SAMPLING";
			define_stochasticSamplingSeamFix = "STOCHASTIC_SAMPLING_MIPMAP_SEAM_FIX";
			define_tonemappingMethod = "TONEMAPPING_METHOD";
		}

		std::string atmScatProjMatUniform;
		std::string modelMatUniform;
		std::string viewMatUniform;
		std::string projectionMatUniform;
		std::string viewProjectionMatUniform;
		std::string modelViewMatUniform;
		std::string modelViewProjectionMatUniform;
		std::string transposeViewMatUniform;
		std::string transposeInverseViewMatUniform;
		std::string screenSizeUniform;
		std::string inverseScreenSizeUniform;
		std::string screenNumOfPixelsUniform;
		std::string deltaTimeMSUniform;
		std::string deltaTimeSUniform;
		std::string elapsedTimeUniform;
		std::string gammaUniform;
		std::string alphaCullingUniform;
		std::string alphaThresholdUniform;
		std::string emissiveMultiplierUniform;
		std::string emissiveThresholdUniform;
		std::string heightScaleUniform;
		std::string parallaxMappingNumOfStepsLayersUniform;
		std::string textureTilingFactorUniform;
		std::string stochasticSamplingScaleUniform;
		std::string LODParallaxUniform;
		std::string texelSize;
		std::string numOfTexels;
		std::string mipLevel;
		std::string projPlaneRange;

		std::string ambientLightIntensity;
		std::string dirLightColor;
		std::string dirLightDirection;
		std::string dirLightIntensity;
		std::string numPointLightsUniform;
		std::string numSpotLightsUniform;
		std::string pointLightViewProjectionMatUniform;
		std::string pointLightBuffer;
		std::string spotLightBuffer;
		std::string spotLightViewProjectionMatUniform;
		std::string stencilPassViewProjectionMatUniform;

		std::string dirShadowMapMVPUniform;
		std::string dirShadowMapBiasMVPUniform;

		std::string cameraPosVecUniform;
		std::string cameraTargetVecUniform;
		std::string cameraUpVecUniform;
		std::string cameraRightVecUniform;

		std::string positionMapUniform;
		std::string diffuseMapUniform;
		std::string normalMapUniform;
		std::string emissiveMapUniform;
		std::string matPropertiesMapUniform;
		std::string intermediateMapUniform;
		std::string finalMapUniform;
		std::string depthMapUniform;
		std::string inputColorMapUniform;
		std::string outputColorMapUniform;

		std::string csmBiasScaleUniform;
		std::string csmDepthMapUniform;
		std::string csmPenumbraScaleRange;

		std::string sunGlowTextureUniform;
		std::string skyMapTextureUniform;
		std::string dirShadowMapTextureUniform;
		std::string diffuseTextureUniform;
		std::string normalTextureUniform;
		std::string specularTextureUniform;
		std::string emissiveTextureUniform;
		std::string glossTextureUniform;
		std::string heightTextureUniform;
		std::string combinedTextureUniform;
		std::string averageLuminanceTexture;
		std::string noiseTexture;

		std::string hbaoBlurHorizontalInvResDirection;
		std::string hbaoBlurVerticalInvResDirection;
		std::string hbaoBlurNumOfSamples;
		std::string hbaoBlurSharpness;

		std::string atmIrradianceTextureUniform;
		std::string atmScatteringTextureUniform;
		std::string atmSingleMieScatTextureUniform;
		std::string atmTransmittanceTextureUniform;
		
		std::string bloomTreshold;
		std::string bloomIntensity;
		std::string bloomDirtIntensity;

		std::string inverseLogLuminanceRange;
		std::string logLuminanceRange;
		std::string luminanceMultiplier;
		std::string minLogLuminance;
		std::string tonemapMethod;

		std::string lensFlareDirtTextureUniform;
		std::string lensFlareGhostGradientTextureUniform;
		std::string lensFlareStarburstTextureUniform;

		std::string dynamicEnvMapUniform;
		std::string staticEnvMapUniform;

		std::string fogDensityUniform;
		std::string fogColorUniform;
		std::string billboardScaleUniform;
		std::string depthTypeUniform;

		std::string AODataSetBuffer;
		std::string atmScatParamBuffer;
		std::string CSMDataSetBuffer;
		std::string eyeAdaptionRateUniform;
		std::string eyeAdaptionIntBrightnessUniform;
		std::string HDRSSBuffer;
		std::string lensFlareParametersBuffer;
		std::string materialDataBuffer;
		std::string SSAOSampleBuffer;

		std::string testMatUniform;
		std::string testVecUniform;
		std::string testFloatUniform;

		// Shader #define variable names
		std::string define_alpha_discard;
		std::string define_fxaa;
		std::string define_fxaa_edge_threshold_min;
		std::string define_fxaa_edge_threshold_max;
		std::string define_fxaa_iterations;
		std::string define_fxaa_subpixel_quality;
		std::string define_maxNumOfPointLights;
		std::string define_maxNumOfSpotLights;
		std::string define_normalMapCompression;
		std::string define_numOfCascades;
		std::string define_numOfMaterialTypes;
		std::string define_numOfPCFSamples;
		std::string define_parallaxMapping;
		std::string define_parallaxMappingMethod;
		std::string define_shadowMapping;
		std::string define_stochasticSampling;
		std::string define_stochasticSamplingSeamFix;
		std::string define_tonemappingMethod;
	};
	struct TextureVariables
	{
		TextureVariables()
		{
			default_texture = "default.png";
			default_emissive_texture = "default_emissive.png";
			default_height_texture = "default_height.png";
			default_normal_texture = "default_normal.png";
			default_RMHA_texture = "default_RMHA.png";
			default_specular_intensity = 1.0f;
			default_specular_power = 32.0f;
			diffuse_texture_format = GL_RGBA;
			normal_texture_format = GL_RGB;
			emissive_texture_format = GL_RGBA;
			roughness_texture_format = GL_R;
			metalness_texture_format = GL_R;
			height_texture_format = GL_R;
			ambientOcclusion_texture_format = GL_R;
			RMHAO_texture_format = GL_RGBA;
			gl_texture_anisotropy = 16;
			gl_texture_magnification = GL_LINEAR;
			gl_texture_minification = GL_LINEAR;
			gl_texture_magnification_mipmap = GL_LINEAR;
			gl_texture_minification_mipmap = GL_LINEAR_MIPMAP_LINEAR;
			number_of_mipmaps = 50;
			texture_compression_format_rgb = TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA;
			texture_compression_format_rgba = TextureDataFormat::TextureDataFormat_COMPRESSED_BPTC_RGBA;
			texture_compression_format_normal = TextureDataFormat::TextureDataFormat_COMPRESSED_RGTC2_RG;
			texture_downsample_max_resolution = 1024;
			texture_downsample_scale = 1;
			generate_mipmaps = true;
			texture_compression = true;
			texture_normal_compression = true;
			texture_downsample = true;
		}

		std::string default_texture;
		std::string default_emissive_texture;
		std::string default_height_texture;
		std::string default_normal_texture;
		std::string default_RMHA_texture;
		float default_specular_intensity;
		float default_specular_power;
		int diffuse_texture_format;
		int normal_texture_format;
		int emissive_texture_format;
		int roughness_texture_format;
		int metalness_texture_format;
		int height_texture_format;
		int ambientOcclusion_texture_format;
		int RMHAO_texture_format;
		int gl_texture_anisotropy;
		int gl_texture_magnification;
		int gl_texture_minification;
		int gl_texture_magnification_mipmap;
		int gl_texture_minification_mipmap;
		int number_of_mipmaps;
		int texture_compression_format_rgb;
		int texture_compression_format_rgba;
		int texture_compression_format_normal;
		int texture_downsample_scale;
		int texture_downsample_max_resolution;
		bool generate_mipmaps;
		bool texture_compression;
		bool texture_normal_compression;
		bool texture_downsample;
	};
	struct WindowVariables
	{
		WindowVariables()
		{
			name = "Praxis3D";
			default_display = 0;
			window_position_x = 0;
			window_position_y = 0;
			window_size_fullscreen_x = 1920;
			window_size_fullscreen_y = 1080;
			window_size_windowed_x = 800;
			window_size_windowed_y = 600;
			borderless = false;
			fullscreen = false;
			fullscreen_borderless = true;
			maximized = false;
			mouse_captured = true;
			mouse_release_on_lost_focus = true;
			resizable = true;
			vertical_sync = true;
			window_in_focus = true;
			window_position_centered = true;
		}

		std::string name;
		int default_display;
		int window_position_x;
		int window_position_y;
		int window_size_fullscreen_x;
		int window_size_fullscreen_y;
		int window_size_windowed_x;
		int window_size_windowed_y;
		bool borderless;
		bool fullscreen;
		bool fullscreen_borderless;
		bool maximized;
		bool mouse_captured;
		bool mouse_release_on_lost_focus;
		bool resizable;
		bool vertical_sync;
		bool window_in_focus;
		bool window_position_centered;
	};

	const inline static AudioVariables		&audioVar()			{ return m_audioVar;		}
	const inline static ComponentVariables	&componentVar()		{ return m_componentVar;	}
	const inline static ConfigFileVariables	&configFileVar()	{ return m_configFileVar;	}
	const inline static EngineVariables		&engineVar()		{ return m_engineVar;		}
	const inline static FramebfrVariables	&getFramebfrVar()	{ return m_framebfrVar;		}
	const inline static GameplayVariables	&gameplayVar()		{ return m_gameplayVar;		}
	const inline static GraphicsVariables	&graphicsVar()		{ return m_graphicsVar;		}
	const inline static GUIVariables		&GUIVar()			{ return m_GUIVar;			}
	const inline static InputVariables		&inputVar()			{ return m_inputVar;		}
	const inline static ModelVariables		&modelVar()			{ return m_modelVar;		}
	const inline static ObjectPoolVariables &objectPoolVar()	{ return m_objPoolVar;		}
	const inline static PathsVariables		&filepathVar()		{ return m_filepathVar;		}
	const inline static PhysicsVariables	&physicsVar()		{ return m_physicsVar;		}
	const inline static RendererVariables	&rendererVar()		{ return m_rendererVar;		}
	const inline static ScriptVariables		&scriptVar()		{ return m_scriptVar;		}
	const inline static ShaderVariables		&shaderVar()		{ return m_shaderVar;		}
	const inline static TextureVariables	&textureVar()		{ return m_textureVar;		}
	const inline static WindowVariables		&windowVar()		{ return m_windowVar;		}

	// Register all config variables, so we can search through them later
	static void init();
	static ErrorCode loadFromFile(const std::string &p_filename);
	static ErrorCode saveToFile(const std::string &p_filename);

private:
	class Variable
	{
	public:
		Variable(std::string p_name, size_t p_mapKey, int *p_int) : m_varType(VariableType::intType), m_name(p_name), m_mapKey(p_mapKey), m_valueChanged(false)
		{
			m_variable.intPtr = p_int;
		}
		Variable(std::string p_name, size_t p_mapKey, bool *p_bool) : m_varType(VariableType::boolType), m_name(p_name), m_mapKey(p_mapKey), m_valueChanged(false)
		{
			m_variable.boolPtr = p_bool;
		}
		Variable(std::string p_name, size_t p_mapKey, float *p_float) : m_varType(VariableType::floatType), m_name(p_name), m_mapKey(p_mapKey), m_valueChanged(false)
		{
			m_variable.floatPtr = p_float;
		}
		Variable(std::string p_name, size_t p_mapKey, std::string *p_string) : m_varType(VariableType::stringType), m_name(p_name), m_mapKey(p_mapKey), m_valueChanged(false)
		{
			m_variable.stringPtr = p_string;
		}
		~Variable() { }

		inline bool operator==(const size_t p_mapKey)		{ return (m_mapKey == p_mapKey); }
		inline bool operator==(const std::string &p_name)	{ return (m_name == p_name);	 }

		inline bool valueChanged() { return m_valueChanged; }

		const inline std::string &getName() { return m_name; }
		inline std::string toString()
		{
			std::string returnString = m_name + " ";

			switch(m_varType)
			{
			case Config::Variable::intType:
				returnString += Utilities::toString(*m_variable.intPtr);
				break;
			case Config::Variable::boolType:
				returnString += Utilities::toString(*m_variable.boolPtr);
				break;
			case Config::Variable::floatType:
				returnString += Utilities::toString(*m_variable.floatPtr);
				break;
			case Config::Variable::stringType:
				returnString += *m_variable.stringPtr;
				break;
			}

			return returnString;
		}

		void setVariable(std::string &p_variable)
		{
			m_valueChanged = true;

			switch(m_varType)
			{
			case Config::Variable::intType:
				*m_variable.intPtr = std::atoi(p_variable.c_str());
				break;
			case Config::Variable::boolType:
				*m_variable.boolPtr = (p_variable == "1" || p_variable == "true" || p_variable == "True" || p_variable == "TRUE");
				break;
			case Config::Variable::floatType:
				*m_variable.floatPtr = (float)std::atof(p_variable.c_str());
				break;
			case Config::Variable::stringType:
				*m_variable.stringPtr = p_variable;
				break;
			}
		}
	
	private:
		enum VariableType
		{
			intType,
			boolType,
			floatType,
			stringType
		};
		union
		{
			int *intPtr;
			bool *boolPtr;
			float *floatPtr;
			std::string *stringPtr;
		} m_variable;

		VariableType m_varType;
		std::string m_name;
		bool m_valueChanged;
		size_t m_mapKey;
	};

	static AudioVariables		m_audioVar;
	static ComponentVariables	m_componentVar;
	static ConfigFileVariables	m_configFileVar;
	static EngineVariables		m_engineVar;
	static FramebfrVariables	m_framebfrVar;
	static GameplayVariables	m_gameplayVar;
	static GraphicsVariables	m_graphicsVar;
	static GUIVariables			m_GUIVar;
	static InputVariables		m_inputVar;
	static ModelVariables		m_modelVar;
	static ObjectPoolVariables	m_objPoolVar;
	static PathsVariables		m_filepathVar;
	static PhysicsVariables		m_physicsVar;
	static RendererVariables	m_rendererVar;
	static ScriptVariables		m_scriptVar;
	static ShaderVariables		m_shaderVar;
	static TextureVariables		m_textureVar;
	static WindowVariables		m_windowVar;

	static std::vector<Variable> m_variables;
	static std::unordered_map<std::string, size_t> m_hashTable;

	static const std::vector<Variable>::size_type m_varVectorOffset = 1;

	static void setVariable(std::string p_name, std::string p_variable);

	inline static AudioVariables		&setAudioVar()		{ return m_audioVar;		}
	inline static ComponentVariables	&setComponentVar()	{ return m_componentVar;	}
	inline static ConfigFileVariables	&setConfigFileVar()	{ return m_configFileVar;	}
	inline static EngineVariables		&setEngineVar()		{ return m_engineVar;		}
	inline static FramebfrVariables		&setFramebfrVar()	{ return m_framebfrVar;		}
	inline static GameplayVariables		&setGameplayVar()	{ return m_gameplayVar;		}
	inline static GraphicsVariables		&setGraphicsVar()	{ return m_graphicsVar;		}
	inline static GUIVariables			&setGUIVar()		{ return m_GUIVar;			}
	inline static InputVariables		&setInputVar()		{ return m_inputVar;		}
	inline static ModelVariables		&setModelVar()		{ return m_modelVar;		}
	inline static PathsVariables		&setFilepathVar()	{ return m_filepathVar;		}
	inline static PhysicsVariables		&setPhysicsVar()	{ return m_physicsVar;		}
	inline static RendererVariables		&setRendererVar()	{ return m_rendererVar;		}
	inline static ScriptVariables		&setScriptVar()		{ return m_scriptVar;		}
	inline static ShaderVariables		&setShaderVar()		{ return m_shaderVar;		}
	inline static TextureVariables		&setTextureVar()	{ return m_textureVar;		}
	inline static WindowVariables		&setWindowVar()		{ return m_windowVar;		}
};
