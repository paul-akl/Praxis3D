#pragma once

#include <GL\glew.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "ErrorCodes.h"
#include "EnumFactory.h"
#include "Utilities.h"

typedef unsigned int BitMask;

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
	Code(Scripting,) \
	Code(NumberOfSystems,) 
	DECLARE_ENUM(TypeID, TYPEID)

	const static std::string SystemNames[NumberOfSystems] =
	{
		GetString(Graphics),
		GetString(Scripting)
	};
	
	namespace Types
	{
		static const BitMask All = static_cast<BitMask>(-1);
		static const BitMask Max = 32;
	}

	namespace Changes
	{
		namespace Generic
		{
			static const BitMask CreateObject	= (1 << 0);
			static const BitMask DeleteObject	= (1 << 1);
			static const BitMask ExtendObject	= (1 << 2);
			static const BitMask UnextendObject = (1 << 3);
			static const BitMask Name			= (1 << 4);
			static const BitMask All			= CreateObject | DeleteObject | ExtendObject;
		}
		namespace Spacial
		{
			static const BitMask Position		= (1 << 5);
			static const BitMask Rotation		= (1 << 6);
			static const BitMask Scale			= (1 << 7);
			static const BitMask ModelMatrix	= (1 << 8);
			static const BitMask All			= Position | Rotation | Scale | ModelMatrix;
		}
		namespace Graphics
		{
			static const BitMask Target		= (1 << 9);
			static const BitMask UpVector	= (1 << 10);
			static const BitMask Lighting	= (1 << 11);
			static const BitMask All		= Target | UpVector | Lighting;
		}
		namespace Physics
		{

		}
		namespace Audio
		{

		}
		namespace Scripting
		{

		}

		static const BitMask Link = (1 << 29);

		static const BitMask None = 0;
		static const BitMask All = static_cast<BitMask>(-1);
	}
}

namespace Properties
{
	#define PROPERTYID(Code) \
	Code(Null, = 0) \
	/* General */ \
	Code(ArrayEntry,) \
	Code(Default,) \
	Code(Filename,) \
	Code(Index,) \
	Code(Keybindings,) \
	Code(LoadInBackground,) \
	Code(Name,) \
	Code(Objects,) \
	Code(Scene,) \
	Code(Systems,) \
	Code(Type,) \
	/* Geometry */ \
	Code(OffsetPosition,) \
	Code(OffsetRotation,) \
	Code(Position,) \
	Code(Rotation,) \
	Code(Scale,) \
	/* Graphics */ \
	Code(AlphaThreshold, ) \
	Code(AmbientOcclusion, ) \
	Code(Attenuation,) \
	Code(Camera,) \
	Code(Color,) \
	Code(CombinedTexture,) \
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
	Code(Height,) \
	Code(HeightScale,) \
	Code(Intensity,) \
	Code(Lighting,) \
	Code(Materials,) \
	Code(Metalness,) \
	Code(Models,) \
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
	Code(RMHAO,) \
	Code(Roughness,) \
	Code(Shaders,) \
	Code(ShaderPoolSize,) \
	Code(ShaderGraphicsObject,) \
	Code(ShaderModelObject,) \
	Code(SpotLight,) \
	Code(SpotLightPoolSize,) \
	Code(Static,) \
	Code(TessControlShader,) \
	Code(TessEvaluationShader,) \
	Code(TextureTilingFactor,) \
	Code(VertexShader,) \
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
	/* Scripting */ \
	Code(Angle,) \
	Code(Axis,) \
	Code(BaseUIScript,) \
	Code(DayOfYear,) \
	Code(DebugMoveScript,) \
	Code(DebugRotateScript,) \
	Code(DebugUIScript,) \
	Code(FreeCamera,) \
	Code(Latitude,) \
	Code(Longitude,) \
	Code(LowerLimit,) \
	Code(InputScript,) \
	Code(Hours,) \
	Code(KeyCode,) \
	Code(KeyName,) \
	Code(Minutes,) \
	Code(Radius,) \
	Code(Scripting,) \
	Code(Seconds,) \
	Code(SolarTimeScript,) \
	Code(Speed,) \
	Code(SprintSpeed,) \
	Code(TimeMultiplier,) \
	Code(UpperLimit,) \
	Code(WorldEditScript,) \
	/* Window */ \
	Code(Fullscreen,) \
	Code(MouseCapture,) \
	Code(VerticalSync,) \
	Code(WindowTitle,) \
	/* End of property IDs */ \
	Code(NumberOfPropertyIDs,) 
	DECLARE_ENUM(PropertyID, PROPERTYID)

	const static std::string PropertyNames[PropertyID::NumberOfPropertyIDs] =
	{
		GetString(Null),
		GetString(ArrayEntry),
		GetString(Default),
		GetString(Filename),
		GetString(Index),
		GetString(Keybindings),
		GetString(LoadInBackground),
		GetString(Name),
		GetString(Objects),
		GetString(Scene),
		GetString(Systems),
		GetString(Type),
		GetString(OffsetPosition),
		GetString(OffsetRotation),
		GetString(Position),
		GetString(Rotation),
		GetString(Scale),
		GetString(AlphaThreshold),
		GetString(AmbientOcclusion),
		GetString(Attenuation),
		GetString(Camera),
		GetString(Color),
		GetString(CombinedTexture),
		GetString(CutoffAngle),
		GetString(Diffuse),
		GetString(Direction),
		GetString(DirectionalLight),
		GetString(Emissive),
		GetString(EnvironmentMapDynamic),
		GetString(EnvironmentMapObject),
		GetString(FragmentShader),
		GetString(GeometryShader),
		GetString(Graphics),
		GetString(Height),
		GetString(HeightScale),
		GetString(Intensity),
		GetString(Lighting),
		GetString(Materials),
		GetString(Metalness),
		GetString(Models),
		GetString(ModelObject),
		GetString(ModelPoolSize),
		GetString(NegativeX),
		GetString(NegativeY),
		GetString(NegativeZ),
		GetString(Normal),
		GetString(ParallaxHeightScale),
		GetString(PointLight),
		GetString(PointLightPoolSize),
		GetString(PositiveX),
		GetString(PositiveY),
		GetString(PositiveZ),
		GetString(PostProcess),
		GetString(RMHAO),
		GetString(Roughness),
		GetString(Shaders),
		GetString(ShaderPoolSize),
		GetString(ShaderGraphicsObject),
		GetString(ShaderModelObject),
		GetString(SpotLight),
		GetString(SpotLightPoolSize),
		GetString(Static),
		GetString(TessControlShader),
		GetString(TessEvaluationShader),
		GetString(TextureTilingFactor),
		GetString(VertexShader),
		GetString(BackwardKey),
		GetString(CenterKey),
		GetString(CloseKey),
		GetString(DebugCaptureMouseKey),
		GetString(DebugFullscreenKey),
		GetString(DebugVertSyncKey),
		GetString(DownKey),
		GetString(ForwardKey),
		GetString(LeftKey),
		GetString(LeftStrafeKey),
		GetString(ModifierKey),
		GetString(NextKey),
		GetString(PreviousKey),
		GetString(RightKey),
		GetString(RightStrafeKey),
		GetString(SaveKey),
		GetString(SprintKey),
		GetString(UpKey),
		GetString(ObjectLinks),
		GetString(Observer),
		GetString(Subject),
		GetString(Angle),
		GetString(Axis),
		GetString(BaseUIScript),
		GetString(DayOfYear),
		GetString(DebugMoveScript),
		GetString(DebugRotateScript),
		GetString(DebugUIScript),
		GetString(FreeCamera),
		GetString(Latitude),
		GetString(Longitude),
		GetString(LowerLimit),
		GetString(InputScript),
		GetString(Hours),
		GetString(KeyCode),
		GetString(KeyName),
		GetString(Minutes),
		GetString(Radius),
		GetString(Scripting),
		GetString(Seconds),
		GetString(SolarTimeScript),
		GetString(Speed),
		GetString(SprintSpeed),
		GetString(TimeMultiplier),
		GetString(UpperLimit),
		GetString(WorldEditScript),
		GetString(Fullscreen),
		GetString(MouseCapture),
		GetString(VerticalSync),
		GetString(WindowTitle)
	};


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
		if(p_value > Properties::PropertyID::Null && p_value < Properties::PropertyID::NumberOfPropertyIDs)
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
			if(Properties::PropertyNames[i] == p_value)
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
	friend class RendererFrontend;
	friend class Window;
public:
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
			gl_context_major_version = 3;
			gl_context_minor_version = 3;
			smoothing_tick_samples = 100;
			running = true;
		}

		int change_ctrl_cml_notify_list_reserv;
		int change_ctrl_grain_size;
		int change_ctrl_notify_list_reserv;
		int change_ctrl_oneoff_notify_list_reserv;
		int change_ctrl_subject_list_reserv;
		int delta_time_divider;
		int gl_context_major_version;
		int gl_context_minor_version;
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

			gl_final_buffer_internal_format = GL_RGB16F;
			gl_final_buffer_texture_format = GL_RGB;
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

			gl_final_buffer_min_filter_HDR = GL_NEAREST_MIPMAP_NEAREST;
			gl_final_buffer_min_filter = GL_NEAREST;
			gl_final_buffer_mag_filter = GL_NEAREST;
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
			double_buffering = true;
			eye_adaption = true;
			multisampling = true;
			alpha_size = 8;
			current_resolution_x = 0;
			current_resolution_y = 0;
			dir_shadow_res_x = 2048;
			dir_shadow_res_y = 2048;
			max_num_point_lights = 50;
			max_num_spot_lights = 50;
			multisample_buffers = 1;
			multisample_samples = 1;
			rendering_res_x = 1600;
			rendering_res_y = 900;
			alpha_threshold = 0.0f;
			emissive_threshold = 0.01f;
			eye_adaption_rate = 0.5f;
			fog_color_x = 0.55f;
			fog_color_y = 0.55f;
			fog_color_z = 0.55f;
			fog_density = 0.003f;
			fov = 60.0f;
			gamma = 2.2f;
			light_atten_constant = 0.0f;
			light_atten_linear = 0.0f;
			light_atten_quadratic = 1.0f;
			light_color_r = 1.0f;
			light_color_g = 1.0f;
			light_color_b = 1.0f;
			LOD_prallax_mapping = 100.0f;
			height_scale = 0.0f;
			texture_tiling_factor = 1.0f;
			z_far = 8000.0f;
			z_near = 0.1f;
		}

		bool double_buffering;
		bool eye_adaption;
		bool multisampling;
		int alpha_size;
		int current_resolution_x;
		int current_resolution_y;
		int dir_shadow_res_x;
		int dir_shadow_res_y;
		int max_num_point_lights;
		int max_num_spot_lights;
		int multisample_buffers;
		int multisample_samples;
		int rendering_res_x;
		int rendering_res_y;
		float alpha_threshold;
		float emissive_threshold;
		float eye_adaption_rate;
		float fog_color_x;
		float fog_color_y;
		float fog_color_z;
		float fog_density;
		float fov;
		float gamma;
		float light_atten_constant;
		float light_atten_linear;
		float light_atten_quadratic;
		float light_color_r;
		float light_color_g;
		float light_color_b;
		float LOD_prallax_mapping;
		float height_scale;
		float texture_tiling_factor;
		float z_far;
		float z_near;
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
			down_key = 89;
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
			up_editor_key = 75;
			up_key = 95;
			vsync_key = 68;
			mouse_filter = false;
			mouse_warp_mode = false;
			mouse_jaw = 0.022f;
			mouse_pitch = 0.022f;
			mouse_pitch_clip = 1.2f;
			mouse_sensitivity = 3.0f;
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
			model_object_pool_size = 20;
			point_light_pool_size = 50;
			shader_object_pool_size = 10;
			spot_light_pool_size = 25;
		}

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
			shader_path = "Data\\Shaders\\";
			sound_path = "Data\\Sounds\\";
			texture_path = "Data\\Materials\\";
		}

		std::string config_path;
		std::string map_path;
		std::string model_path;
		std::string object_path;
		std::string shader_path;
		std::string sound_path;
		std::string texture_path;
	};
	struct RendererVariables
	{
		RendererVariables()
		{
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
			blur_pass_vert_shader = "blurPass.vert";
			blur_pass_frag_shader = "blurPass.frag";
			light_pass_vert_shader = "lightPass.vert";
			light_pass_frag_shader = "lightPass.frag";
			final_pass_vert_shader = "finalPass.vert";
			final_pass_frag_shader = "finalPass.frag";
			reflection_pass_vert_shader = "reflectionPass.vert";
			reflection_pass_frag_shader = "reflectionPass.frag";
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
		std::string blur_pass_vert_shader;
		std::string blur_pass_frag_shader;
		std::string light_pass_vert_shader;
		std::string light_pass_frag_shader;
		std::string final_pass_vert_shader;
		std::string final_pass_frag_shader;
		std::string reflection_pass_vert_shader;
		std::string reflection_pass_frag_shader;
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
	struct ShaderVariables
	{
		ShaderVariables()
		{
			modelMatUniform = "modelMat";
			viewMatUniform = "viewMat";
			projectionMatUniform = "projMat";
			viewProjectionMatUniform = "viewProjMat";
			modelViewMatUniform = "modelViewMat";
			modelViewProjectionMatUniform = "MVP";
			screenSizeUniform = "screenSize";
			deltaTimeMSUniform = "deltaTimeMS";
			deltaTimeSUniform = "deltaTimeS";
			elapsedTimeUniform = "elapsedTime";
			gammaUniform = "gamma";
			alphaCullingUniform = "alphaCulling";
			alphaThresholdUniform = "alphaThreshold";
			emissiveThresholdUniform = "emissiveThreshold";
			heightScaleUniform = "heightScale";
			textureTilingFactorUniform = "textureTilingFactor";
			LODParallaxUniform = "parallaxMappingLOD";

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
			blurMapUniform = "blurMap";
			finalMapUniform = "finalColorMap";

			sunGlowTextureUniform = "sunGlowMap";
			skyMapTextureUniform = "skyMap";
			dirShadowMapTextureUniform = "dirShadowMap";
			diffuseTextureUniform = "diffuseTexture";
			normalTextureUniform = "normalTexture";
			specularTextureUniform = "specularTexture";
			emissiveTextureUniform = "emissiveTexture";
			blurTextureUniform = "blurTexture";
			glossTextureUniform = "glossTexture";
			heightTextureUniform = "heightTexture";
			combinedTextureUniform = "combinedTexture";

			dynamicEnvMapUniform = "dynamicEnvMap";
			staticEnvMapUniform = "staticEnvMap";

			fogDensityUniform = "fogDensity";
			fogColorUniform = "fogColor";
			billboardScaleUniform = "billboardScale";
			depthTypeUniform = "depthType";

			eyeAdaptionRateUniform = "eyeAdaptionRate";
			HDRSSBuffer = "HDRBuffer";

			testMatUniform = "testMat";
			testVecUniform = "testVec";
			testFloatUniform = "testFloat";
		}

		std::string modelMatUniform;
		std::string viewMatUniform;
		std::string projectionMatUniform;
		std::string viewProjectionMatUniform;
		std::string modelViewMatUniform;
		std::string modelViewProjectionMatUniform;
		std::string screenSizeUniform;
		std::string deltaTimeMSUniform;
		std::string deltaTimeSUniform;
		std::string elapsedTimeUniform;
		std::string gammaUniform;
		std::string alphaCullingUniform;
		std::string alphaThresholdUniform;
		std::string emissiveThresholdUniform;
		std::string heightScaleUniform;
		std::string textureTilingFactorUniform;
		std::string LODParallaxUniform;

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
		std::string blurMapUniform;
		std::string finalMapUniform;

		std::string sunGlowTextureUniform;
		std::string skyMapTextureUniform;
		std::string dirShadowMapTextureUniform;
		std::string diffuseTextureUniform;
		std::string normalTextureUniform;
		std::string specularTextureUniform;
		std::string emissiveTextureUniform;
		std::string blurTextureUniform;
		std::string glossTextureUniform;
		std::string heightTextureUniform;
		std::string combinedTextureUniform;

		std::string dynamicEnvMapUniform;
		std::string staticEnvMapUniform;

		std::string fogDensityUniform;
		std::string fogColorUniform;
		std::string billboardScaleUniform;
		std::string depthTypeUniform;

		std::string eyeAdaptionRateUniform;
		std::string HDRSSBuffer;

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
			gl_texture_minification = GL_LINEAR_MIPMAP_LINEAR;
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

	const inline static ConfigFileVariables	&configFileVar()	{ return m_configFileVar;	}
	const inline static EngineVariables		&engineVar()		{ return m_engineVar;		}
	const inline static FramebfrVariables	&getFramebfrVar()	{ return m_framebfrVar;		}
	const inline static GameplayVariables	&gameplayVar()		{ return m_gameplayVar;		}
	const inline static GraphicsVariables	&graphicsVar()		{ return m_graphicsVar;		}
	const inline static InputVariables		&inputVar()			{ return m_inputVar;		}
	const inline static ModelVariables		&modelVar()			{ return m_modelVar;		}
	const inline static ObjectPoolVariables &objectPoolVar()	{ return m_objPoolVar;		}
	const inline static PathsVariables		&filepathVar()		{ return m_filepathVar;		}
	const inline static RendererVariables	&rendererVar()		{ return m_rendererVar;		}
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

	static ConfigFileVariables	m_configFileVar;
	static EngineVariables		m_engineVar;
	static FramebfrVariables	m_framebfrVar;
	static GameplayVariables	m_gameplayVar;
	static GraphicsVariables	m_graphicsVar;
	static InputVariables		m_inputVar;
	static ModelVariables		m_modelVar;
	static ObjectPoolVariables	m_objPoolVar;
	static PathsVariables		m_filepathVar;
	static RendererVariables	m_rendererVar;
	static ShaderVariables		m_shaderVar;
	static TextureVariables		m_textureVar;
	static WindowVariables		m_windowVar;

	static std::vector<Variable> m_variables;
	static std::unordered_map<std::string, size_t> m_hashTable;

	static const std::vector<Variable>::size_type m_varVectorOffset = 1;

	static void setVariable(std::string p_name, std::string p_variable);

	inline static ConfigFileVariables	&setConfigFileVar()	{ return m_configFileVar;	}
	inline static EngineVariables		&setEngineVar()		{ return m_engineVar;		}
	inline static FramebfrVariables		&setFramebfrVar()	{ return m_framebfrVar;		}
	inline static GameplayVariables		&setGameplayVar()	{ return m_gameplayVar;		}
	inline static GraphicsVariables		&setGraphicsVar()	{ return m_graphicsVar;		}
	inline static InputVariables		&setInputVar()		{ return m_inputVar;		}
	inline static ModelVariables		&setModelVar()		{ return m_modelVar;		}
	inline static PathsVariables		&setFilepathVar()	{ return m_filepathVar;		}
	inline static RendererVariables		&setRendererVar()	{ return m_rendererVar;		}
	inline static ShaderVariables		&setShaderVar()		{ return m_shaderVar;		}
	inline static TextureVariables		&setTextureVar()	{ return m_textureVar;		}
	inline static WindowVariables		&setWindowVar()		{ return m_windowVar;		}
};
