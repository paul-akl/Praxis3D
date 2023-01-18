#pragma once

#include <climits>
#include <GL\glew.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "ErrorCodes.h"
#include "EnumFactory.h"
#include "Utilities.h"

typedef unsigned __int64 BitMask;

// Tests if the given bitmask contains the given flag; returns true if the flag bits are present in the bitmask
constexpr bool CheckBitmask(const BitMask p_bitmask, const BitMask p_flag) { return ((p_bitmask & p_flag) == p_flag); }

//#define ever ;;

//#define GL_LINEAR_MIPMAP_LINEAR 0x2703
//#define GL_LINEAR 0x2601
//#define GL_LESS 0x0201
//#define GL_BACK 0x0405

namespace Systems
{
	#define TYPEID(Code) \
	Code(Null, = -1) \
	Code(Graphics,) \
	Code(GUI,) \
	Code(Physics,) \
	Code(Script,) \
	Code(World,) \
	Code(NumberOfSystems,) 
	DECLARE_ENUM(TypeID, TYPEID)

	const static std::string SystemNames[NumberOfSystems] =
	{
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
		}

		namespace Generic
		{
			static constexpr BitMask CreateObject		= Changes::Type::Generic + Changes::Common::Shared1;
			static constexpr BitMask DeleteObject		= Changes::Type::Generic + Changes::Common::Shared2;
			static constexpr BitMask ExtendObject		= Changes::Type::Generic + Changes::Common::Shared3;
			static constexpr BitMask UnextendObject		= Changes::Type::Generic + Changes::Common::Shared4;
			static constexpr BitMask Name				= Changes::Type::Generic + Changes::Common::Shared5;
			static constexpr BitMask Link				= Changes::Type::Generic + Changes::Common::Shared6;
			static constexpr BitMask All				= CreateObject | DeleteObject | ExtendObject | Name | Link;
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

			static constexpr BitMask AllLocalNoTransform	= LocalPosition | LocalRotation | LocalScale;
			static constexpr BitMask AllWorldNoTransform	= WorldPosition | WorldRotation | WorldScale;
			static constexpr BitMask AllLocal				= AllLocalNoTransform | LocalTransform | LocalTransformNoScale;
			static constexpr BitMask AllWorld				= AllWorldNoTransform | WorldTransform | WorldTransformNoScale;
			static constexpr BitMask All					= AllLocal | AllWorld;
		}
		namespace Audio
		{

		}
		namespace Graphics
		{
			static constexpr BitMask Lighting				= Changes::Type::Graphics + Changes::Common::Shared21;
			static constexpr BitMask Camera					= Changes::Type::Graphics + Changes::Common::Shared20;

			static constexpr BitMask Target					= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared1;
			static constexpr BitMask UpVector				= Changes::Type::Graphics + Changes::Graphics::Camera + Changes::Common::Shared2;
			static constexpr BitMask AllCamera				= Target | UpVector;

			static constexpr BitMask LightEnabled			= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared3;
			static constexpr BitMask Color					= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared4;
			static constexpr BitMask CutoffAngle			= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared5;
			static constexpr BitMask Direction				= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared6;
			static constexpr BitMask Intensity				= Changes::Type::Graphics + Changes::Graphics::Lighting + Changes::Common::Shared7;
			static constexpr BitMask AllLighting			= LightEnabled | Color | CutoffAngle | Direction | Intensity;

			static constexpr BitMask All					= AllCamera | AllLighting;
		}
		namespace GUI
		{
			static constexpr BitMask Sequence				= Changes::Type::GUI + Changes::Common::Shared1;
			static constexpr BitMask Placholder				= Changes::Type::GUI + Changes::Common::Shared2;
			static constexpr BitMask All					= Sequence | Placholder;
		}
		namespace Physics
		{
			static constexpr BitMask Placholder1			= Changes::Type::Physics + Changes::Common::Shared1;
			static constexpr BitMask Placholder2			= Changes::Type::Physics + Changes::Common::Shared2;
			static constexpr BitMask All = Placholder1 | Placholder2;
		}
		namespace Script
		{
			static constexpr BitMask Placholder				= Changes::Type::GUI + Changes::Common::Shared1;
			static constexpr BitMask All					= Placholder;
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
	Code(ArrayEntry,) \
	Code(Components,) \
	Code(Default,) \
	Code(Filename,) \
	Code(Index,) \
	Code(Keybindings,) \
	Code(LoadInBackground,) \
	Code(Name,) \
	Code(Objects,) \
	Code(ObjectPoolSize,) \
	Code(Scene,) \
	Code(Systems,) \
	Code(Type,) \
	Code(Value,) \
	Code(Variables,) \
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
	Code(AmbientOcclusion, ) \
	Code(Attenuation,) \
	Code(Camera,) \
	Code(CameraComponent,) \
	Code(Color,) \
	Code(CombinedTexture,) \
	Code(ComputeShader,) \
	Code(CutoffAngle,) \
	Code(Diffuse,) \
	Code(Direction,) \
	Code(DirectionalLight,) \
	Code(Emissive,) \
	Code(EnvironmentMapDynamic,) \
	Code(EnvironmentMapObject,) \
	Code(FragmentShader,) \
	Code(GeometryShader,) \
	Code(Graphics,) \
	Code(GraphicsObject,) \
	Code(Height,) \
	Code(HeightScale,) \
	Code(Intensity,) \
	Code(LightComponent,) \
	Code(Lighting,) \
	Code(Materials,) \
	Code(Metalness,) \
	Code(Meshes,) \
	Code(Models,) \
	Code(ModelComponent,) \
	Code(ModelObject,) \
	Code(ModelPoolSize,) \
	Code(NegativeX,) \
	Code(NegativeY,) \
	Code(NegativeZ,) \
	Code(Normal,) \
	Code(ParallaxHeightScale,) \
	Code(PointLight,) \
	Code(PointLightPoolSize,) \
	Code(PositiveX,) \
	Code(PositiveY,) \
	Code(PositiveZ,) \
	Code(PostProcess,) \
	Code(Renderer,) \
	Code(Rendering,) \
	Code(RMHAO,) \
	Code(Roughness,) \
	Code(Shaders,) \
	Code(ShaderComponent,) \
	Code(ShaderPoolSize,) \
	Code(ShaderGraphicsObject,) \
	Code(ShaderModelObject,) \
	Code(SpotLight,) \
	Code(SpotLightPoolSize,) \
	Code(Static,) \
	Code(TessControlShader,) \
	Code(TessEvaluationShader,) \
	Code(TextureTilingFactor,) \
	Code(TextureScale,) \
	Code(VertexShader,) \
	/* GUI */ \
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
	Code(Restitution,) \
	Code(RigidBody,) \
	Code(RigidBodyComponent,) \
	Code(Size,) \
	Code(Sphere,) \
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
	Code(GameObject,) \
	Code(ID,) \
	Code(Parent,) \
	Code(SpatialComponent,) \
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
	friend class DebugUIScript;
	friend class DeferredRenderer;
	friend class ErrorHandler;
	friend class LuaScript;
	friend class RendererFrontend;
	friend class Window;
public:
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
			change_ctrl_oneoff_notify_list_reserv = 64;
			change_ctrl_subject_list_reserv = 8192;
			delta_time_divider = 1000;
			glsl_version = 430;
			gl_context_major_version = 3;
			gl_context_minor_version = 3;
			object_directory_init_pool_size = 1000;
			smoothing_tick_samples = 100;
			running = true;
		}

		int change_ctrl_cml_notify_list_reserv;
		int change_ctrl_grain_size;
		int change_ctrl_notify_list_reserv;
		int change_ctrl_oneoff_notify_list_reserv;
		int change_ctrl_subject_list_reserv;
		int delta_time_divider;
		int glsl_version;
		int gl_context_major_version;
		int gl_context_minor_version;
		int object_directory_init_pool_size;
		int smoothing_tick_samples;
		bool running;
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

			gl_buffers_min_filter = GL_NEAREST;
			gl_buffers_mag_filter = GL_NEAREST;
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
			camera_freelook_speed = 10.0f;
		}

		std::string default_map;
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
			bloom_blur_passes = 5;
			bloom_downscale_limit = 10;
			bloom_mipmap_limit = 16;
			current_resolution_x = 0;
			current_resolution_y = 0;
			dir_shadow_res_x = 2048;
			dir_shadow_res_y = 2048;
			lens_flare_blur_passes = 5;
			lens_flare_ghost_count = 4;
			max_num_point_lights = 50;
			max_num_spot_lights = 50;
			multisample_buffers = 1;
			multisample_samples = 1;
			rendering_res_x = 1600;
			rendering_res_y = 900;
			tonemap_method = 6;
			alpha_threshold = 0.0f;
			bloom_intensity = 1.0f;
			bloom_knee = 0.1f;
			bloom_threshold = 1.5f;
			bloom_dirt_intensity = 1.0f;
			emissive_multiplier = 10.0f;
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
			LOD_prallax_mapping = 100.0f;
			luminance_range_min = 0.004f;
			luminance_range_max = 11.3f;
			height_scale = 0.0f;
			texture_tiling_factor = 1.0f;
			z_far = 8000.0f;
			z_near = 0.1f;
		}

		bool bloom_enabled;
		bool double_buffering;
		bool eye_adaption;
		bool multisampling;
		int alpha_size;
		int bloom_blur_passes;
		int bloom_downscale_limit;
		int bloom_mipmap_limit;
		int current_resolution_x;
		int current_resolution_y;
		int dir_shadow_res_x;
		int dir_shadow_res_y;
		int lens_flare_blur_passes;
		int lens_flare_ghost_count;
		int max_num_point_lights;
		int max_num_spot_lights;
		int multisample_buffers;
		int multisample_samples;
		int rendering_res_x;
		int rendering_res_y;
		int tonemap_method;
		float alpha_threshold;
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
		float LOD_prallax_mapping;
		float luminance_range_min;
		float luminance_range_max;
		float height_scale;
		float texture_tiling_factor;
		float z_far;
		float z_near;
	};
	struct GUIVariables
	{
		GUIVariables()
		{
			gui_render = true;
			gui_dark_style = true;
			gui_sequence_array_reserve_size = 50;
		}
		bool gui_render;
		bool gui_dark_style;
		int gui_sequence_array_reserve_size;
	};
	struct InputVariables
	{
		InputVariables()
		{
			back_key = 42;
			backward_editor_key = 90;
			backward_key = 22;
			center_key = 93;
			clip_mouse_key = 67;
			close_window_key = 41;
			debug_1_key = 58;
			debug_2_key = 59;
			down_editor_key = 89;
			down_key = 6;
			escape_key = 41;
			forward_editor_key = 96;
			forward_key = 26;
			fullscreen_key = 66;
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
			vsync_key = 68;
			mouse_filter = false;
			mouse_warp_mode = false;
			mouse_jaw = 0.005f;
			mouse_pitch = 0.005f;
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
			joinIdenticalVertices = false;
			makeLeftHanded = false;
			triangulate = true;
			removeComponent = false;
			genNormals = true;
			genSmoothNormals = true;
			genUVCoords = true;
			optimizeMeshes = true;
			optimizeGraph = false;
		}

		bool calcTangentSpace;
		bool joinIdenticalVertices;
		bool makeLeftHanded;
		bool triangulate;
		bool removeComponent;
		bool genNormals;
		bool genSmoothNormals;
		bool genUVCoords;
		bool optimizeMeshes;
		bool optimizeGraph;
	};
	struct ObjectPoolVariables
	{
		ObjectPoolVariables()
		{
			object_pool_size = 50;
			model_object_pool_size = 20;
			point_light_pool_size = 50;
			shader_object_pool_size = 10;
			spot_light_pool_size = 25;
		}

		int object_pool_size;
		int model_object_pool_size;
		int point_light_pool_size;
		int shader_object_pool_size;
		int spot_light_pool_size;
	};
	struct PathsVariables
	{
		PathsVariables()
		{
			config_path = "Data\\";
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
		std::string map_path;
		std::string model_path;
		std::string object_path;
		std::string prefab_path;
		std::string script_path;
		std::string shader_path;
		std::string sound_path;
		std::string texture_path;
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
			lens_flare_dirt_texture = "DirtMaskTexture.png";
			lens_flare_ghost_gradient_texture = "p3d_lensFlareGhostColorGradient.png";
			lens_flare_starburst_texture = "p3d_lensFlareStarburst.png";
			dir_light_quad_offset_x = 0.0f;
			dir_light_quad_offset_y = 0.0f;
			dir_light_quad_offset_z = 0.0f;
			dir_light_quad_rotation_x = 180.0f;
			dir_light_quad_rotation_y = 0.0f;
			dir_light_quad_rotation_z = 0.0f;
			depth_test_func = GL_LESS;
			face_culling_mode = GL_BACK;
			heightmap_combine_channel = 3;
			heightmap_combine_texture = 1;
			max_num_point_lights = 50;
			max_num_spot_lights = 10;
			objects_loaded_per_frame = 1;
			shader_pool_size = 10;
			depth_test = true;
			face_culling = true;
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
		float dir_light_quad_offset_x;
		float dir_light_quad_offset_y;
		float dir_light_quad_offset_z;
		float dir_light_quad_rotation_x;
		float dir_light_quad_rotation_y;
		float dir_light_quad_rotation_z;
		int depth_test_func;
		int face_culling_mode;
		int heightmap_combine_channel;
		int heightmap_combine_texture;
		int max_num_point_lights;
		int max_num_spot_lights;
		int objects_loaded_per_frame;
		int shader_pool_size;
		bool depth_test;
		bool face_culling;
	};
	struct ScriptVariables
	{
		ScriptVariables()
		{
			iniFunctionName = "init";
			updateFunctionName = "update";
			createObjectFunctionName = "create";
			userTypeTableName = "Types";
		}

		std::string iniFunctionName;
		std::string updateFunctionName;
		std::string createObjectFunctionName;
		std::string userTypeTableName;
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
			screenSizeUniform = "screenSize";
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
			textureTilingFactorUniform = "textureTilingFactor";
			LODParallaxUniform = "parallaxMappingLOD";
			texelSize = "texelSize";
			numOfTexels = "numOfTexels";
			mipLevel = "mipLevel";

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
			inputColorMapUniform = "inputColorMap";
			outputColorMapUniform = "outputColorMap";

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

			atmIrradianceTextureUniform = "atmIrradianceTexture";
			atmScatteringTextureUniform = "atmScatteringTexture";
			atmSingleMieScatTextureUniform = "atmSingleMieTexture";
			atmTransmittanceTextureUniform = "atmTransmitTexture";

			bloomTreshold = "bloomTreshold";
			bloomIntensity = "bloomIntensity";
			bloomDirtIntensity = "bloomDirtIntensity";

			inverseLogLuminanceRange = "inverseLogLuminanceRange";
			logLuminanceRange = "logLuminanceRange";
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

			eyeAdaptionRateUniform = "eyeAdaptionRate";
			eyeAdaptionIntBrightnessUniform = "eyeAdaptionIntBrightness";
			HDRSSBuffer = "HDRBuffer";
			atmScatParamBuffer = "AtmScatParametersBuffer";
			lensFlareParametersBuffer = "LensFlareParametersBuffer";

			testMatUniform = "testMat";
			testVecUniform = "testVec";
			testFloatUniform = "testFloat";
		}

		std::string atmScatProjMatUniform;
		std::string modelMatUniform;
		std::string viewMatUniform;
		std::string projectionMatUniform;
		std::string viewProjectionMatUniform;
		std::string modelViewMatUniform;
		std::string modelViewProjectionMatUniform;
		std::string transposeViewMatUniform;
		std::string screenSizeUniform;
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
		std::string textureTilingFactorUniform;
		std::string LODParallaxUniform;
		std::string texelSize;
		std::string numOfTexels;
		std::string mipLevel;

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
		std::string inputColorMapUniform;
		std::string outputColorMapUniform;

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

		std::string atmIrradianceTextureUniform;
		std::string atmScatteringTextureUniform;
		std::string atmSingleMieScatTextureUniform;
		std::string atmTransmittanceTextureUniform;
		
		std::string bloomTreshold;
		std::string bloomIntensity;
		std::string bloomDirtIntensity;

		std::string inverseLogLuminanceRange;
		std::string logLuminanceRange;
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

		std::string eyeAdaptionRateUniform;
		std::string eyeAdaptionIntBrightnessUniform;
		std::string HDRSSBuffer;
		std::string atmScatParamBuffer;
		std::string lensFlareParametersBuffer;

		std::string testMatUniform;
		std::string testVecUniform;
		std::string testFloatUniform;
	};
	struct TextureVariables
	{
		TextureVariables()
		{
			default_texture = "default.png";
			default_emissive_texture = "default_emissive.png";
			default_height_texture = "default_height.png";
			default_normal_texture = "default_normal.png";
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
			generate_mipmaps = true;
		}

		std::string default_texture;
		std::string default_emissive_texture;
		std::string default_height_texture;
		std::string default_normal_texture;
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
		bool generate_mipmaps;
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
			fullscreen = false;
			fullscreen_borderless = true;
			mouse_captured = true;
			mouse_release_on_lost_focus = true;
			resizable = true;
			vertical_sync = true;
			window_in_focus = true;
		}

		std::string name;
		int default_display;
		int window_position_x;
		int window_position_y;
		int window_size_fullscreen_x;
		int window_size_fullscreen_y;
		int window_size_windowed_x;
		int window_size_windowed_y;
		bool fullscreen;
		bool fullscreen_borderless;
		bool mouse_captured;
		bool mouse_release_on_lost_focus;
		bool resizable;
		bool vertical_sync;
		bool window_in_focus;
	};

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
		Variable(std::string p_name, size_t p_mapKey, std::string *p_string) : m_varType(VariableType::stringType), m_name(p_name), m_mapKey(p_mapKey)
		{
			m_variable.stringPtr = p_string;
		}
		~Variable() { }

		inline bool operator==(const size_t p_mapKey)		 { return (m_mapKey == p_mapKey); }
		inline bool operator==(const std::string &p_name) { return (m_name == p_name); }

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
	static RendererVariables	m_rendererVar;
	static ScriptVariables		m_scriptVar;
	static ShaderVariables		m_shaderVar;
	static TextureVariables		m_textureVar;
	static WindowVariables		m_windowVar;

	static std::vector<Variable> m_variables;
	static std::unordered_map<std::string, size_t> m_hashTable;

	static const std::vector<Variable>::size_type m_varVectorOffset = 1;

	static void setVariable(std::string p_name, std::string p_variable);

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
	inline static RendererVariables		&setRendererVar()	{ return m_rendererVar;		}
	inline static ScriptVariables		&setScriptVar()		{ return m_scriptVar;		}
	inline static ShaderVariables		&setShaderVar()		{ return m_shaderVar;		}
	inline static TextureVariables		&setTextureVar()	{ return m_textureVar;		}
	inline static WindowVariables		&setWindowVar()		{ return m_windowVar;		}
};
