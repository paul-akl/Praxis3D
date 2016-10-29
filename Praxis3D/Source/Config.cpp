#include <fstream>

#include "Config.h"
#include "ErrorHandlerLocator.h"
#include "Loaders.h"

namespace Systems
{
	DEFINE_ENUM(TypeID, TYPEID)
}
namespace Properties
{
	DEFINE_ENUM(PropertyID, PROPERTYID)
}

// Predefined variables for "AddVariablePredef" macro
#define VEC_PREDEF m_variables
#define HASH_PREDEF m_hashTable
#define OFFSET_PREDEF m_varVectorOffset

// Macros used to simplify the method of adding configuration variables
// Also used to add variable name as a string, without the need of hand-writing it,
// and making sure it spelled the same as the variable name (makes code editing easier)
#define AddVariable(VECTOR, HASH, KEY, STRUCT, VAR) HASH[#VAR] = KEY; VECTOR.push_back(Variable(#VAR, KEY, &STRUCT.VAR))

// A version of the "AddVariable" macro, with 3 of the 5 parameters predefined
#define AddVariablePredef(STRUCT, VAR) AddVariable(VEC_PREDEF, HASH_PREDEF, VEC_PREDEF.size() + OFFSET_PREDEF, STRUCT, VAR)

Config::ConfigFileVariables	Config::m_configFileVar;
Config::EngineVariables		Config::m_engineVar;
Config::FramebfrVariables	Config::m_framebfrVar;
Config::GameplayVariables	Config::m_gameplayVar;
Config::GraphicsVariables	Config::m_graphicsVar;
Config::InputVariables		Config::m_inputVar;
Config::ModelVariables		Config::m_modelVar;
Config::ObjectPoolVariables	Config::m_objPoolVar;
Config::PathsVariables		Config::m_filepathVar;
Config::RendererVariables	Config::m_rendererVar;
Config::ShaderVariables		Config::m_shaderVar;
Config::TextureVariables	Config::m_textureVar;
Config::WindowVariables		Config::m_windowVar;

std::vector<Config::Variable> Config::m_variables;
std::unordered_map<std::string, size_t> Config::m_hashTable;

void Config::init()
{
	// Add variables and their names to internal containers, so they could be checked / names matched later.
	// Note: not all the variables are assigned to containers, as some are not meant to be loaded from config file.

	// Engine variables
	AddVariablePredef(m_engineVar, change_ctrl_cml_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_grain_size);
	AddVariablePredef(m_engineVar, change_ctrl_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_oneoff_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_subject_list_reserv);
	AddVariablePredef(m_engineVar, delta_time_divider);
	AddVariablePredef(m_engineVar, gl_context_major_version);
	AddVariablePredef(m_engineVar, gl_context_minor_version);
	AddVariablePredef(m_engineVar, smoothing_tick_samples);

	// Frame-buffer variables
	AddVariablePredef(m_framebfrVar, gl_position_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_position_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_position_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_diffuse_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_diffuse_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_diffuse_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_emissive_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_emissive_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_emissive_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_normal_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_normal_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_normal_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_depth_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_depth_buffer_texture_type);
	AddVariablePredef(m_framebfrVar, gl_depth_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_buffers_min_filter);
	AddVariablePredef(m_framebfrVar, gl_buffers_mag_filter);
	AddVariablePredef(m_framebfrVar, gl_buffers_wrap_s_method);
	AddVariablePredef(m_framebfrVar, gl_buffers_wrap_t_method);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_min_filter);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_mag_filter);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_wrap_s_method);
	AddVariablePredef(m_framebfrVar, gl_blur_buffer_wrap_t_method);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_min_filter);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_mag_filter);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_s_method);
	AddVariablePredef(m_framebfrVar, gl_final_buffer_t_method);

	// Game-play variables
	AddVariablePredef(m_gameplayVar, default_map);
	AddVariablePredef(m_gameplayVar, camera_freelook_speed);

	// Graphics variables
	AddVariablePredef(m_graphicsVar, double_buffering);
	AddVariablePredef(m_graphicsVar, multisampling);
	AddVariablePredef(m_graphicsVar, alpha_size);
	AddVariablePredef(m_graphicsVar, dir_shadow_res_x);
	AddVariablePredef(m_graphicsVar, dir_shadow_res_y);
	AddVariablePredef(m_graphicsVar, max_num_point_lights);
	AddVariablePredef(m_graphicsVar, max_num_spot_lights);
	AddVariablePredef(m_graphicsVar, multisample_buffers);
	AddVariablePredef(m_graphicsVar, multisample_samples);
	AddVariablePredef(m_graphicsVar, alpha_threshold);
	AddVariablePredef(m_graphicsVar, emissive_threshold);
	AddVariablePredef(m_graphicsVar, parallax_height_scale);
	AddVariablePredef(m_graphicsVar, fog_color_x);
	AddVariablePredef(m_graphicsVar, fog_color_y);
	AddVariablePredef(m_graphicsVar, fog_color_z);
	AddVariablePredef(m_graphicsVar, fog_density);
	AddVariablePredef(m_graphicsVar, fov);
	AddVariablePredef(m_graphicsVar, gamma);
	AddVariablePredef(m_graphicsVar, light_atten_constant);
	AddVariablePredef(m_graphicsVar, light_atten_linear);
	AddVariablePredef(m_graphicsVar, light_atten_quadratic);
	AddVariablePredef(m_graphicsVar, light_color_r);
	AddVariablePredef(m_graphicsVar, light_color_g);
	AddVariablePredef(m_graphicsVar, light_color_b);
	AddVariablePredef(m_graphicsVar, texture_tiling_factor);
	AddVariablePredef(m_graphicsVar, z_far);
	AddVariablePredef(m_graphicsVar, z_near);
	
	// Input variables
	AddVariablePredef(m_inputVar, back_key);
	AddVariablePredef(m_inputVar, backward_editor_key);
	AddVariablePredef(m_inputVar, backward_key);
	AddVariablePredef(m_inputVar, center_key);
	AddVariablePredef(m_inputVar, clip_mouse_key);
	AddVariablePredef(m_inputVar, close_window_key);
	AddVariablePredef(m_inputVar, debug_1_key);
	AddVariablePredef(m_inputVar, debug_2_key);
	AddVariablePredef(m_inputVar, down_editor_key);
	AddVariablePredef(m_inputVar, down_key);
	AddVariablePredef(m_inputVar, escape_key);
	AddVariablePredef(m_inputVar, forward_editor_key);
	AddVariablePredef(m_inputVar, forward_key);
	AddVariablePredef(m_inputVar, fullscreen_key);
	AddVariablePredef(m_inputVar, jump_key);
	AddVariablePredef(m_inputVar, left_editor_key);
	AddVariablePredef(m_inputVar, left_strafe_key);
	AddVariablePredef(m_inputVar, next_editor_key);
	AddVariablePredef(m_inputVar, modifier_editor_key);
	AddVariablePredef(m_inputVar, next_editor_key);
	AddVariablePredef(m_inputVar, num_preallocated_keybinds);
	AddVariablePredef(m_inputVar, previous_editor_key);
	AddVariablePredef(m_inputVar, right_editor_key);
	AddVariablePredef(m_inputVar, right_strafe_key);
	AddVariablePredef(m_inputVar, save_editor_key);
	AddVariablePredef(m_inputVar, sprint_key);
	AddVariablePredef(m_inputVar, up_editor_key);
	AddVariablePredef(m_inputVar, up_key);
	AddVariablePredef(m_inputVar, vsync_key);
	AddVariablePredef(m_inputVar, mouse_filter);
	AddVariablePredef(m_inputVar, mouse_warp_mode);
	AddVariablePredef(m_inputVar, mouse_jaw);
	AddVariablePredef(m_inputVar, mouse_pitch);
	AddVariablePredef(m_inputVar, mouse_pitch_clip);
	AddVariablePredef(m_inputVar, mouse_sensitivity);

	// Model variables
	AddVariablePredef(m_modelVar, calcTangentSpace);
	AddVariablePredef(m_modelVar, joinIdenticalVertices);
	AddVariablePredef(m_modelVar, makeLeftHanded);
	AddVariablePredef(m_modelVar, triangulate);
	AddVariablePredef(m_modelVar, removeComponent);
	AddVariablePredef(m_modelVar, genNormals);
	AddVariablePredef(m_modelVar, genSmoothNormals);
	AddVariablePredef(m_modelVar, genUVCoords);
	AddVariablePredef(m_modelVar, optimizeMeshes);
	AddVariablePredef(m_modelVar, optimizeGraph);

	// Object pool variables
	AddVariablePredef(m_objPoolVar, model_object_pool_size);
	AddVariablePredef(m_objPoolVar, point_light_pool_size);
	AddVariablePredef(m_objPoolVar, shader_object_pool_size);
	AddVariablePredef(m_objPoolVar, spot_light_pool_size);

	// File-path variables
	AddVariablePredef(m_filepathVar, config_path);
	AddVariablePredef(m_filepathVar, map_path);
	AddVariablePredef(m_filepathVar, model_path);
	AddVariablePredef(m_filepathVar, object_path);
	AddVariablePredef(m_filepathVar, shader_path);
	AddVariablePredef(m_filepathVar, sound_path);
	AddVariablePredef(m_filepathVar, texture_path);
	
	// Renderer variables
	AddVariablePredef(m_rendererVar, dir_light_vert_shader);
	AddVariablePredef(m_rendererVar, dir_light_frag_shader);
	AddVariablePredef(m_rendererVar, point_light_vert_shader);
	AddVariablePredef(m_rendererVar, point_light_frag_shader);
	AddVariablePredef(m_rendererVar, spot_light_vert_shader);
	AddVariablePredef(m_rendererVar, spot_light_frag_shader);
	AddVariablePredef(m_rendererVar, dir_light_quad);
	AddVariablePredef(m_rendererVar, point_light_sphere);
	AddVariablePredef(m_rendererVar, spot_light_cone);
	AddVariablePredef(m_rendererVar, stencil_pass_vert_shader);
	AddVariablePredef(m_rendererVar, stencil_pass_frag_shader);
	AddVariablePredef(m_rendererVar, geometry_pass_vert_shader);
	AddVariablePredef(m_rendererVar, geometry_pass_frag_shader);
	AddVariablePredef(m_rendererVar, geom_billboard_vert_shader);
	AddVariablePredef(m_rendererVar, geom_billboard_frag_shader);
	AddVariablePredef(m_rendererVar, geom_billboard_goem_shader);
	AddVariablePredef(m_rendererVar, gaussian_blur_vertical_frag_shader);
	AddVariablePredef(m_rendererVar, gaussian_blur_vertical_vert_shader);
	AddVariablePredef(m_rendererVar, gaussian_blur_horizontal_frag_shader);
	AddVariablePredef(m_rendererVar, gaussian_blur_horizontal_vert_shader);
	AddVariablePredef(m_rendererVar, light_pass_vert_shader);
	AddVariablePredef(m_rendererVar, light_pass_frag_shader);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_x);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_y);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_z);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_x);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_y);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_z);
	AddVariablePredef(m_rendererVar, depth_test_func);
	AddVariablePredef(m_rendererVar, face_culling_mode);
	AddVariablePredef(m_rendererVar, heightmap_combine_channel);
	AddVariablePredef(m_rendererVar, heightmap_combine_texture);
	AddVariablePredef(m_rendererVar, max_num_point_lights);
	AddVariablePredef(m_rendererVar, max_num_spot_lights);
	AddVariablePredef(m_rendererVar, objects_loaded_per_frame);
	AddVariablePredef(m_rendererVar, shader_pool_size);
	AddVariablePredef(m_rendererVar, depth_test);
	AddVariablePredef(m_rendererVar, face_culling);

	// Shader variables
	AddVariablePredef(m_shaderVar, modelMatUniform);
	AddVariablePredef(m_shaderVar, viewMatUniform);
	AddVariablePredef(m_shaderVar, projectionMatUniform);
	AddVariablePredef(m_shaderVar, viewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, modelViewMatUniform);
	AddVariablePredef(m_shaderVar, modelViewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, screenSizeUniform);
	AddVariablePredef(m_shaderVar, elapsedTimeUniform);
	AddVariablePredef(m_shaderVar, gammaUniform);
	AddVariablePredef(m_shaderVar, alphaCullingUniform);
	AddVariablePredef(m_shaderVar, alphaThresholdUniform);
	AddVariablePredef(m_shaderVar, emissiveThresholdUniform);
	AddVariablePredef(m_shaderVar, parallaxHeightScale);
	AddVariablePredef(m_shaderVar, textureTilingFactorUniform);
	AddVariablePredef(m_shaderVar, dirLightColor);
	AddVariablePredef(m_shaderVar, dirLightDirection);
	AddVariablePredef(m_shaderVar, dirLightIntensity);
	AddVariablePredef(m_shaderVar, numPointLightsUniform);
	AddVariablePredef(m_shaderVar, numSpotLightsUniform);
	AddVariablePredef(m_shaderVar, pointLightViewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, pointLightBuffer);
	AddVariablePredef(m_shaderVar, spotLightBuffer);
	AddVariablePredef(m_shaderVar, spotLightViewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, stencilPassViewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, dirShadowMapMVPUniform);
	AddVariablePredef(m_shaderVar, dirShadowMapBiasMVPUniform);
	AddVariablePredef(m_shaderVar, cameraPosVecUniform);
	AddVariablePredef(m_shaderVar, cameraUpVecUniform);
	AddVariablePredef(m_shaderVar, cameraRightVecUniform);
	AddVariablePredef(m_shaderVar, positionMapUniform);
	AddVariablePredef(m_shaderVar, diffuseMapUniform);
	AddVariablePredef(m_shaderVar, normalMapUniform);
	AddVariablePredef(m_shaderVar, emissiveMapUniform);
	AddVariablePredef(m_shaderVar, blurMapUniform);
	AddVariablePredef(m_shaderVar, sunGlowTextureUniform);
	AddVariablePredef(m_shaderVar, skyMapTextureUniform);
	AddVariablePredef(m_shaderVar, dirShadowMapTextureUniform);
	AddVariablePredef(m_shaderVar, diffuseTextureUniform);
	AddVariablePredef(m_shaderVar, normalTextureUniform);
	AddVariablePredef(m_shaderVar, specularTextureUniform);
	AddVariablePredef(m_shaderVar, emissiveTextureUniform);
	AddVariablePredef(m_shaderVar, glossTextureUniform);
	AddVariablePredef(m_shaderVar, heightTextureUniform);
	AddVariablePredef(m_shaderVar, fogDensityUniform);
	AddVariablePredef(m_shaderVar, fogColorUniform);
	AddVariablePredef(m_shaderVar, billboardScaleUniform);
	AddVariablePredef(m_shaderVar, depthTypeUniform);
	AddVariablePredef(m_shaderVar, testMatUniform);
	AddVariablePredef(m_shaderVar, testVecUniform);
	AddVariablePredef(m_shaderVar, testFloatUniform);

	// Texture variables
	AddVariablePredef(m_textureVar, default_texture);
	AddVariablePredef(m_textureVar, default_emissive_texture);
	AddVariablePredef(m_textureVar, default_height_texture);
	AddVariablePredef(m_textureVar, default_normal_texture);
	AddVariablePredef(m_textureVar, default_specular_intensity);
	AddVariablePredef(m_textureVar, default_specular_power);
	AddVariablePredef(m_textureVar, gl_texture_anisotropy);
	AddVariablePredef(m_textureVar, gl_texture_magnification);
	AddVariablePredef(m_textureVar, gl_texture_minification);
	AddVariablePredef(m_textureVar, number_of_mipmaps);
	AddVariablePredef(m_textureVar, generate_mipmaps);

	// Window variables
	AddVariablePredef(m_windowVar, name);
	AddVariablePredef(m_windowVar, default_display);
	AddVariablePredef(m_windowVar, window_position_x);
	AddVariablePredef(m_windowVar, window_position_y);
	AddVariablePredef(m_windowVar, window_size_fullscreen_x);
	AddVariablePredef(m_windowVar, window_size_fullscreen_y);
	AddVariablePredef(m_windowVar, window_size_windowed_x);
	AddVariablePredef(m_windowVar, window_size_windowed_y);
	AddVariablePredef(m_windowVar, fullscreen);
	AddVariablePredef(m_windowVar, fullscreen_borderless);
	AddVariablePredef(m_windowVar, mouse_captured);
	AddVariablePredef(m_windowVar, mouse_release_on_lost_focus);
	AddVariablePredef(m_windowVar, resizable);
	AddVariablePredef(m_windowVar, vertical_sync);
}
ErrorCode Config::loadFromFile(const std::string &p_filename)
{
	std::ifstream configFile(p_filename, std::ifstream::in);
	std::string singleWord;
	ErrorCode returnCode = ErrorCode::Success;

	if(configFile.fail())
	{
		returnCode = ErrorCode::Ifstream_failed;
	}
	else
	{
		while(!configFile.eof())
		{
			// Get next word
			configFile >> singleWord;

			// Match the word in the hash table
			auto &tableEntry = m_hashTable.find(singleWord);

			// If match was found
			if(tableEntry != m_hashTable.end())
			{
				// Get map key and next word (that will be converted to value)
				auto mapKey = tableEntry->second;
				configFile >> singleWord;

				// Pass the word as a value
				m_variables[mapKey - m_varVectorOffset].setVariable(singleWord);

				// DEBUGGING: log an info error about modified variable
				ErrHandlerLoc::get().log(Info, Source_Config, m_variables[mapKey - m_varVectorOffset].toString());
			}
		}
	}

	configFile.close();

	// Check boundaries of some values that are integer representations of enums
	if(m_rendererVar.heightmap_combine_channel < TextureColorChannelOffset::ColorOffset_Red ||
	   m_rendererVar.heightmap_combine_channel >= TextureColorChannelOffset::ColorOffset_NumChannels)
		m_rendererVar.heightmap_combine_channel = TextureColorChannelOffset::ColorOffset_Alpha;

	if(m_rendererVar.heightmap_combine_texture < Model::ModelMaterialType::ModelMat_diffuse ||
	   m_rendererVar.heightmap_combine_texture >= Model::ModelMaterialType::NumOfModelMaterials)
		m_rendererVar.heightmap_combine_texture = Model::ModelMaterialType::ModelMat_normal;

	return returnCode;
}
ErrorCode Config::saveToFile(const std::string &p_filename)
{
	std::string fileContents;

	for(std::vector<Variable>::size_type i = 0, size = m_variables.size(); i < size; i++)
	{
		// Only add variables that have been changed (if they haven't no need to add them,
		// since they are initialized to that value by default
		if(m_variables[i].valueChanged())
		{
			fileContents += m_variables[i].toString() + "\n";
		}
	}

	return ErrorCode::Success;
}

void Config::setVariable(std::string p_name, std::string p_variable)
{
	// Match the name in the hash table
	auto &tableEntry = m_hashTable.find(p_name);

	// If match was found
	if(tableEntry != m_hashTable.end())
	{
		// Get map key
		auto mapKey = tableEntry->second;

		// Pass the word as a value
		m_variables[mapKey - m_varVectorOffset].setVariable(p_variable);

		// DEBUGGING: log an info error about modified variable
		ErrHandlerLoc::get().log(Info, Source_Config, m_variables[mapKey - m_varVectorOffset].toString());
	}
}