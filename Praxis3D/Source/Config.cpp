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

Config::AudioVariables		Config::m_audioVar;
Config::ComponentVariables	Config::m_componentVar;
Config::ConfigFileVariables	Config::m_configFileVar;
Config::EngineVariables		Config::m_engineVar;
Config::FramebfrVariables	Config::m_framebfrVar;
Config::GameplayVariables	Config::m_gameplayVar;
Config::GraphicsVariables	Config::m_graphicsVar;
Config::GUIVariables		Config::m_GUIVar;
Config::InputVariables		Config::m_inputVar;
Config::ModelVariables		Config::m_modelVar;
Config::ObjectPoolVariables	Config::m_objPoolVar;
Config::PathsVariables		Config::m_filepathVar;
Config::PhysicsVariables	Config::m_physicsVar;
Config::RendererVariables	Config::m_rendererVar;
Config::ScriptVariables		Config::m_scriptVar;
Config::ShaderVariables		Config::m_shaderVar;
Config::TextureVariables	Config::m_textureVar;
Config::WindowVariables		Config::m_windowVar;

std::vector<Config::Variable> Config::m_variables;
std::unordered_map<std::string, std::size_t> Config::m_hashTable;

void Config::init()
{
	// Add variables and their names to internal containers, so they could be checked / names matched later.
	// Note: not all the variables are assigned to containers, as some are not meant to be loaded from config file.

	// Audio Variables
	AddVariablePredef(m_audioVar, impact_impulse_param_divider);
	AddVariablePredef(m_audioVar, impact_impulse_volume_divider);
	AddVariablePredef(m_audioVar, impact_max_volume_threshold); 
	AddVariablePredef(m_audioVar, impact_soft_hard_threshold);
	AddVariablePredef(m_audioVar, max_impact_volume);
	AddVariablePredef(m_audioVar, volume_ambient);
	AddVariablePredef(m_audioVar, volume_master);
	AddVariablePredef(m_audioVar, volume_music);
	AddVariablePredef(m_audioVar, volume_sfx); 
	AddVariablePredef(m_audioVar, num_audio_channels);
	AddVariablePredef(m_audioVar, bus_name_ambient);
	AddVariablePredef(m_audioVar, bus_name_master);
	AddVariablePredef(m_audioVar, bus_name_music);
	AddVariablePredef(m_audioVar, bus_name_prefix);
	AddVariablePredef(m_audioVar, bus_name_sfx);
	AddVariablePredef(m_audioVar, channel_name_master);
	AddVariablePredef(m_audioVar, default_sound_bank); 
	AddVariablePredef(m_audioVar, default_sound_bank_string);
	AddVariablePredef(m_audioVar, default_impact_sound_bank);
	AddVariablePredef(m_audioVar, pathDelimiter);

	// Component Variables
	AddVariablePredef(m_componentVar, camera_component_name);
	AddVariablePredef(m_componentVar, component_name_separator);
	AddVariablePredef(m_componentVar, light_component_name);
	AddVariablePredef(m_componentVar, lua_component_name);
	AddVariablePredef(m_componentVar, model_component_name);
	AddVariablePredef(m_componentVar, shader_component_name);

	// Engine variables
	AddVariablePredef(m_engineVar, change_ctrl_cml_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_grain_size);
	AddVariablePredef(m_engineVar, change_ctrl_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_oneoff_data_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_oneoff_notify_list_reserv);
	AddVariablePredef(m_engineVar, change_ctrl_subject_list_reserv);
	AddVariablePredef(m_engineVar, delta_time_divider);
	AddVariablePredef(m_engineVar, glsl_version);
	AddVariablePredef(m_engineVar, gl_context_major_version);
	AddVariablePredef(m_engineVar, gl_context_minor_version);
	AddVariablePredef(m_engineVar, loaders_num_of_unload_per_frame);
	AddVariablePredef(m_engineVar, object_directory_init_pool_size);
	AddVariablePredef(m_engineVar, smoothing_tick_samples);
	AddVariablePredef(m_engineVar, task_scheduler_clock_frequency);

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
	AddVariablePredef(m_framebfrVar, gl_mat_properties_buffer_internal_format);
	AddVariablePredef(m_framebfrVar, gl_mat_properties_buffer_texture_format);
	AddVariablePredef(m_framebfrVar, gl_mat_properties_buffer_texture_type);
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
	AddVariablePredef(m_gameplayVar, main_menu_map);
	AddVariablePredef(m_gameplayVar, play_map);
	AddVariablePredef(m_gameplayVar, camera_freelook_speed);

	// Graphics variables
	AddVariablePredef(m_graphicsVar, double_buffering);
	AddVariablePredef(m_graphicsVar, eye_adaption);
	AddVariablePredef(m_graphicsVar, multisampling);
	AddVariablePredef(m_graphicsVar, alpha_size);
	AddVariablePredef(m_graphicsVar, ao_blur_num_of_samples);
	AddVariablePredef(m_graphicsVar, ao_num_of_directions);
	AddVariablePredef(m_graphicsVar, ao_num_of_samples);
	AddVariablePredef(m_graphicsVar, ao_num_of_steps);
	AddVariablePredef(m_graphicsVar, ao_type);
	AddVariablePredef(m_graphicsVar, bloom_blur_passes);
	AddVariablePredef(m_graphicsVar, dir_shadow_res_x);
	AddVariablePredef(m_graphicsVar, dir_shadow_res_y);
	AddVariablePredef(m_graphicsVar, lens_flare_blur_passes);
	AddVariablePredef(m_graphicsVar, lens_flare_ghost_count);
	AddVariablePredef(m_graphicsVar, max_num_point_lights);
	AddVariablePredef(m_graphicsVar, max_num_spot_lights);
	AddVariablePredef(m_graphicsVar, multisample_buffers);
	AddVariablePredef(m_graphicsVar, multisample_samples);
	AddVariablePredef(m_graphicsVar, rendering_res_x);
	AddVariablePredef(m_graphicsVar, rendering_res_y);
	AddVariablePredef(m_graphicsVar, render_to_texture_resolution_x);
	AddVariablePredef(m_graphicsVar, render_to_texture_resolution_y);
	AddVariablePredef(m_graphicsVar, tonemap_method);
	AddVariablePredef(m_graphicsVar, alpha_threshold);
	AddVariablePredef(m_graphicsVar, ambient_intensity_directional);
	AddVariablePredef(m_graphicsVar, ambient_intensity_point);
	AddVariablePredef(m_graphicsVar, ambient_intensity_spot);
	AddVariablePredef(m_graphicsVar, ao_hbao_bias);
	AddVariablePredef(m_graphicsVar, ao_ssao_bias);
	AddVariablePredef(m_graphicsVar, ao_blurSharpness);
	AddVariablePredef(m_graphicsVar, ao_hbao_intensity);
	AddVariablePredef(m_graphicsVar, ao_ssao_intensity);
	AddVariablePredef(m_graphicsVar, ao_hbao_radius);
	AddVariablePredef(m_graphicsVar, ao_ssao_radius);
	AddVariablePredef(m_graphicsVar, bloom_intensity);
	AddVariablePredef(m_graphicsVar, bloom_knee);
	AddVariablePredef(m_graphicsVar, bloom_threshold);
	AddVariablePredef(m_graphicsVar, bloom_dirt_intensity);
	AddVariablePredef(m_graphicsVar, emissive_multiplier);
	AddVariablePredef(m_graphicsVar, emissive_threshold);
	AddVariablePredef(m_graphicsVar, eye_adaption_rate);
	AddVariablePredef(m_graphicsVar, eye_adaption_intended_brightness);
	AddVariablePredef(m_graphicsVar, fog_color_x);
	AddVariablePredef(m_graphicsVar, fog_color_y);
	AddVariablePredef(m_graphicsVar, fog_color_z);
	AddVariablePredef(m_graphicsVar, fog_density);
	AddVariablePredef(m_graphicsVar, fov);
	AddVariablePredef(m_graphicsVar, gamma);
	AddVariablePredef(m_graphicsVar, lens_flare_aspect_ratio);
	AddVariablePredef(m_graphicsVar, lens_flare_chrom_abberration);
	AddVariablePredef(m_graphicsVar, lens_flare_downsample);
	AddVariablePredef(m_graphicsVar, lens_flare_ghost_spacing);
	AddVariablePredef(m_graphicsVar, lens_flare_ghost_threshold);
	AddVariablePredef(m_graphicsVar, lens_flare_halo_radius);
	AddVariablePredef(m_graphicsVar, lens_flare_halo_thickness);
	AddVariablePredef(m_graphicsVar, lens_flare_halo_threshold);
	AddVariablePredef(m_graphicsVar, light_atten_constant);
	AddVariablePredef(m_graphicsVar, light_atten_linear);
	AddVariablePredef(m_graphicsVar, light_atten_quadratic);
	AddVariablePredef(m_graphicsVar, light_color_r);
	AddVariablePredef(m_graphicsVar, light_color_g);
	AddVariablePredef(m_graphicsVar, light_color_b);
	AddVariablePredef(m_graphicsVar, LOD_parallax_mapping);
	AddVariablePredef(m_graphicsVar, luminance_multiplier);
	AddVariablePredef(m_graphicsVar, luminance_range_min);
	AddVariablePredef(m_graphicsVar, luminance_range_max);
	AddVariablePredef(m_graphicsVar, height_scale);
	AddVariablePredef(m_graphicsVar, stochastic_sampling_scale);
	AddVariablePredef(m_graphicsVar, texture_tiling_factor);
	AddVariablePredef(m_graphicsVar, z_far);
	AddVariablePredef(m_graphicsVar, z_near);

	// GUI variables
	AddVariablePredef(m_GUIVar, gui_docking_enabled);
	AddVariablePredef(m_GUIVar, gui_render);
	AddVariablePredef(m_GUIVar, gui_dark_style);
	AddVariablePredef(m_GUIVar, gui_sequence_array_reserve_size);
	AddVariablePredef(m_GUIVar, editor_asset_selection_button_size_multiplier);
	AddVariablePredef(m_GUIVar, editor_asset_texture_button_size_x);
	AddVariablePredef(m_GUIVar, editor_asset_texture_button_size_y);
	AddVariablePredef(m_GUIVar, editor_audio_banks_max_height);
	AddVariablePredef(m_GUIVar, editor_float_slider_speed);
	AddVariablePredef(m_GUIVar, editor_inspector_button_width_multiplier);
	AddVariablePredef(m_GUIVar, editor_lua_variables_max_height);
	AddVariablePredef(m_GUIVar, editor_play_button_size);
	AddVariablePredef(m_GUIVar, editor_render_pass_max_height);
	AddVariablePredef(m_GUIVar, gui_file_dialog_min_size_x); 
	AddVariablePredef(m_GUIVar, gui_file_dialog_min_size_y);
	AddVariablePredef(m_GUIVar, gui_file_dialog_dir_color_R);
	AddVariablePredef(m_GUIVar, gui_file_dialog_dir_color_G);
	AddVariablePredef(m_GUIVar, gui_file_dialog_dir_color_B);
	AddVariablePredef(m_GUIVar, loading_spinner_radius);
	AddVariablePredef(m_GUIVar, loading_spinner_speed);
	AddVariablePredef(m_GUIVar, loading_spinner_thickness);
	AddVariablePredef(m_GUIVar, editor_button_add_texture);
	AddVariablePredef(m_GUIVar, editor_button_add_list_texture);
	AddVariablePredef(m_GUIVar, editor_button_arrow_down_texture);
	AddVariablePredef(m_GUIVar, editor_button_arrow_up_texture);
	AddVariablePredef(m_GUIVar, editor_button_delete_entry_texture); 
	AddVariablePredef(m_GUIVar, editor_button_gui_sequence_texture);
	AddVariablePredef(m_GUIVar, editor_button_guizmo_rotate_texture);
	AddVariablePredef(m_GUIVar, editor_button_guizmo_translate_texture);
	AddVariablePredef(m_GUIVar, editor_button_open_file_texture);
	AddVariablePredef(m_GUIVar, editor_button_open_asset_list_texture);
	AddVariablePredef(m_GUIVar, editor_button_pause_texture); 
	AddVariablePredef(m_GUIVar, editor_button_play_texture);
	AddVariablePredef(m_GUIVar, editor_button_reload_texture);
	AddVariablePredef(m_GUIVar, editor_button_restart_texture);
	AddVariablePredef(m_GUIVar, editor_button_scripting_enabled_texture);
	AddVariablePredef(m_GUIVar, editor_new_entity_name);
	AddVariablePredef(m_GUIVar, gui_editor_window_name);

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
	AddVariablePredef(m_modelVar, genBoundingBoxes);
	AddVariablePredef(m_modelVar, genNormals);
	AddVariablePredef(m_modelVar, genSmoothNormals);
	AddVariablePredef(m_modelVar, genUVCoords);
	AddVariablePredef(m_modelVar, joinIdenticalVertices);
	AddVariablePredef(m_modelVar, makeLeftHanded);
	AddVariablePredef(m_modelVar, optimizeCacheLocality);
	AddVariablePredef(m_modelVar, optimizeGraph);
	AddVariablePredef(m_modelVar, optimizeMeshes);
	AddVariablePredef(m_modelVar, removeComponent);
	AddVariablePredef(m_modelVar, triangulate);

	// Object pool variables
	AddVariablePredef(m_objPoolVar, camera_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, light_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, lua_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, gui_sequence_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, model_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, object_pool_size);
	AddVariablePredef(m_objPoolVar, point_light_pool_size);
	AddVariablePredef(m_objPoolVar, regid_body_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, shader_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, spatial_component_default_pool_size);
	AddVariablePredef(m_objPoolVar, spot_light_pool_size);
	AddVariablePredef(m_objPoolVar, sound_component_default_pool_size); 
	AddVariablePredef(m_objPoolVar, sound_listener_component_default_pool_size); 

	// File-path variables
	AddVariablePredef(m_filepathVar, config_path);
	AddVariablePredef(m_filepathVar, engine_assets_path);
	AddVariablePredef(m_filepathVar, gui_assets_path);
	AddVariablePredef(m_filepathVar, map_path);
	AddVariablePredef(m_filepathVar, model_path);
	AddVariablePredef(m_filepathVar, object_path);
	AddVariablePredef(m_filepathVar, prefab_path);
	AddVariablePredef(m_filepathVar, script_path);
	AddVariablePredef(m_filepathVar, shader_path);
	AddVariablePredef(m_filepathVar, sound_path);
	AddVariablePredef(m_filepathVar, texture_path);

	// Physics variables
	AddVariablePredef(m_physicsVar, applied_impulse_threshold);
	AddVariablePredef(m_physicsVar, life_time_threshold);
	
	// Renderer variables
	AddVariablePredef(m_rendererVar, atm_scattering_ground_vert_shader);
	AddVariablePredef(m_rendererVar, atm_scattering_ground_frag_shader);
	AddVariablePredef(m_rendererVar, atm_scattering_sky_vert_shader);
	AddVariablePredef(m_rendererVar, atm_scattering_sky_frag_shader);
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
	AddVariablePredef(m_rendererVar, hdr_mapping_pass_frag_shader); 
	AddVariablePredef(m_rendererVar, hdr_mapping_pass_vert_shader);
	AddVariablePredef(m_rendererVar, luminance_average_comp_shader);
	AddVariablePredef(m_rendererVar, luminance_histogram_comp_shader);
	AddVariablePredef(m_rendererVar, tonemapping_vert_shader);
	AddVariablePredef(m_rendererVar, tonemapping_frag_shader);
	AddVariablePredef(m_rendererVar, bloom_composite_pass_vert_shader);
	AddVariablePredef(m_rendererVar, bloom_composite_pass_frag_shader);
	AddVariablePredef(m_rendererVar, bloom_downscale_comp_shader);
	AddVariablePredef(m_rendererVar, bloom_upscale_comp_shader);
	AddVariablePredef(m_rendererVar, bloom_composite_pass_vert_shader); 
	AddVariablePredef(m_rendererVar, bloom_composite_pass_frag_shader);
	AddVariablePredef(m_rendererVar, blur_pass_vert_shader);
	AddVariablePredef(m_rendererVar, blur_pass_frag_shader);
	AddVariablePredef(m_rendererVar, csm_pass_frag_shader);
	AddVariablePredef(m_rendererVar, csm_pass_geom_shader);
	AddVariablePredef(m_rendererVar, csm_pass_vert_shader);
	AddVariablePredef(m_rendererVar, hbao_blur_horizontal_frag_shader);
	AddVariablePredef(m_rendererVar, hbao_blur_horizontal_vert_shader);
	AddVariablePredef(m_rendererVar, hbao_blur_vertical_frag_shader);
	AddVariablePredef(m_rendererVar, hbao_blur_vertical_vert_shader);
	AddVariablePredef(m_rendererVar, hbao_pass_frag_shader);
	AddVariablePredef(m_rendererVar, hbao_pass_vert_shader);
	AddVariablePredef(m_rendererVar, lense_flare_comp_pass_vert_shader);
	AddVariablePredef(m_rendererVar, lense_flare_comp_pass_frag_shader);
	AddVariablePredef(m_rendererVar, lense_flare_pass_vert_shader);
	AddVariablePredef(m_rendererVar, lense_flare_pass_frag_shader);
	AddVariablePredef(m_rendererVar, light_pass_csm_vert_shader);
	AddVariablePredef(m_rendererVar, light_pass_csm_frag_shader);
	AddVariablePredef(m_rendererVar, light_pass_vert_shader);
	AddVariablePredef(m_rendererVar, light_pass_frag_shader);
	AddVariablePredef(m_rendererVar, final_pass_vert_shader);
	AddVariablePredef(m_rendererVar, final_pass_frag_shader);
	AddVariablePredef(m_rendererVar, final_pass_vert_shader);
	AddVariablePredef(m_rendererVar, final_pass_frag_shader);
	AddVariablePredef(m_rendererVar, reflection_pass_vert_shader);
	AddVariablePredef(m_rendererVar, reflection_pass_frag_shader);
	AddVariablePredef(m_rendererVar, lens_flare_dirt_texture);
	AddVariablePredef(m_rendererVar, lens_flare_ghost_gradient_texture);
	AddVariablePredef(m_rendererVar, lens_flare_starburst_texture);
	AddVariablePredef(m_rendererVar, ssao_blur_frag_shader);
	AddVariablePredef(m_rendererVar, ssao_blur_vert_shader);
	AddVariablePredef(m_rendererVar, ssao_pass_frag_shader);
	AddVariablePredef(m_rendererVar, ssao_pass_vert_shader);
	AddVariablePredef(m_rendererVar, texture_repetition_noise_texture);
	AddVariablePredef(m_rendererVar, csm_max_shadow_bias);
	AddVariablePredef(m_rendererVar, csm_penumbra_size);
	AddVariablePredef(m_rendererVar, csm_penumbra_size_scale_min);
	AddVariablePredef(m_rendererVar, csm_penumbra_size_scale_max);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_x);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_y);
	AddVariablePredef(m_rendererVar, dir_light_quad_offset_z);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_x);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_y);
	AddVariablePredef(m_rendererVar, dir_light_quad_rotation_z);
	AddVariablePredef(m_rendererVar, parallax_mapping_min_steps);
	AddVariablePredef(m_rendererVar, parallax_mapping_max_steps);
	AddVariablePredef(m_rendererVar, csm_num_of_pcf_samples);
	AddVariablePredef(m_rendererVar, csm_resolution);
	AddVariablePredef(m_rendererVar, depth_test_func);
	AddVariablePredef(m_rendererVar, face_culling_mode);
	AddVariablePredef(m_rendererVar, heightmap_combine_channel);
	AddVariablePredef(m_rendererVar, heightmap_combine_texture);
	AddVariablePredef(m_rendererVar, max_num_point_lights);
	AddVariablePredef(m_rendererVar, max_num_spot_lights);
	AddVariablePredef(m_rendererVar, objects_loaded_per_frame);
	AddVariablePredef(m_rendererVar, parallax_mapping_method);
	AddVariablePredef(m_rendererVar, render_to_texture_buffer);
	AddVariablePredef(m_rendererVar, shader_pool_size);
	AddVariablePredef(m_rendererVar, ssao_num_of_samples);
	AddVariablePredef(m_rendererVar, depth_test);
	AddVariablePredef(m_rendererVar, face_culling);
	AddVariablePredef(m_rendererVar, stochastic_sampling_seam_fix);

	// Script variables
	AddVariablePredef(m_scriptVar, defaultScriptFilename);
	AddVariablePredef(m_scriptVar, iniFunctionName);
	AddVariablePredef(m_scriptVar, updateFunctionName);
	AddVariablePredef(m_scriptVar, createObjectFunctionName);
	AddVariablePredef(m_scriptVar, userTypeTableName);

	// Shader variables
	AddVariablePredef(m_shaderVar, atmScatProjMatUniform);
	AddVariablePredef(m_shaderVar, modelMatUniform);
	AddVariablePredef(m_shaderVar, viewMatUniform);
	AddVariablePredef(m_shaderVar, projectionMatUniform);
	AddVariablePredef(m_shaderVar, viewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, modelViewMatUniform);
	AddVariablePredef(m_shaderVar, modelViewProjectionMatUniform);
	AddVariablePredef(m_shaderVar, transposeViewMatUniform);
	AddVariablePredef(m_shaderVar, transposeInverseViewMatUniform);
	AddVariablePredef(m_shaderVar, screenSizeUniform);
	AddVariablePredef(m_shaderVar, screenNumOfPixelsUniform);
	AddVariablePredef(m_shaderVar, deltaTimeMSUniform);
	AddVariablePredef(m_shaderVar, deltaTimeSUniform);
	AddVariablePredef(m_shaderVar, elapsedTimeUniform);
	AddVariablePredef(m_shaderVar, gammaUniform);
	AddVariablePredef(m_shaderVar, alphaCullingUniform);
	AddVariablePredef(m_shaderVar, alphaThresholdUniform);
	AddVariablePredef(m_shaderVar, emissiveMultiplierUniform);
	AddVariablePredef(m_shaderVar, emissiveThresholdUniform);
	AddVariablePredef(m_shaderVar, heightScaleUniform);
	AddVariablePredef(m_shaderVar, parallaxMappingNumOfStepsLayersUniform);
	AddVariablePredef(m_shaderVar, combinedTextureUniform);
	AddVariablePredef(m_shaderVar, textureTilingFactorUniform);
	AddVariablePredef(m_shaderVar, stochasticSamplingScaleUniform);
	AddVariablePredef(m_shaderVar, LODParallaxUniform);
	AddVariablePredef(m_shaderVar, texelSize);
	AddVariablePredef(m_shaderVar, numOfTexels);
	AddVariablePredef(m_shaderVar, mipLevel);
	AddVariablePredef(m_shaderVar, projPlaneRange);
	AddVariablePredef(m_shaderVar, ambientLightIntensity);
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
	AddVariablePredef(m_shaderVar, matPropertiesMapUniform);
	AddVariablePredef(m_shaderVar, intermediateMapUniform);
	AddVariablePredef(m_shaderVar, finalMapUniform);
	AddVariablePredef(m_shaderVar, depthMapUniform);
	AddVariablePredef(m_shaderVar, inputColorMapUniform);
	AddVariablePredef(m_shaderVar, outputColorMapUniform);
	AddVariablePredef(m_shaderVar, csmDepthMapUniform);
	AddVariablePredef(m_shaderVar, csmPenumbraScaleRange);
	AddVariablePredef(m_shaderVar, sunGlowTextureUniform);
	AddVariablePredef(m_shaderVar, skyMapTextureUniform);
	AddVariablePredef(m_shaderVar, dirShadowMapTextureUniform);
	AddVariablePredef(m_shaderVar, diffuseTextureUniform);
	AddVariablePredef(m_shaderVar, normalTextureUniform);
	AddVariablePredef(m_shaderVar, specularTextureUniform);
	AddVariablePredef(m_shaderVar, emissiveTextureUniform);
	AddVariablePredef(m_shaderVar, glossTextureUniform);
	AddVariablePredef(m_shaderVar, heightTextureUniform);
	AddVariablePredef(m_shaderVar, combinedTextureUniform);
	AddVariablePredef(m_shaderVar, averageLuminanceTexture);
	AddVariablePredef(m_shaderVar, noiseTexture);
	AddVariablePredef(m_shaderVar, hbaoBlurHorizontalInvResDirection);
	AddVariablePredef(m_shaderVar, hbaoBlurVerticalInvResDirection);
	AddVariablePredef(m_shaderVar, hbaoBlurNumOfSamples);
	AddVariablePredef(m_shaderVar, hbaoBlurSharpness);
	AddVariablePredef(m_shaderVar, atmIrradianceTextureUniform);
	AddVariablePredef(m_shaderVar, atmScatteringTextureUniform);
	AddVariablePredef(m_shaderVar, atmSingleMieScatTextureUniform);
	AddVariablePredef(m_shaderVar, atmTransmittanceTextureUniform);
	AddVariablePredef(m_shaderVar, bloomTreshold);
	AddVariablePredef(m_shaderVar, bloomIntensity);
	AddVariablePredef(m_shaderVar, bloomDirtIntensity);
	AddVariablePredef(m_shaderVar, inverseLogLuminanceRange);
	AddVariablePredef(m_shaderVar, logLuminanceRange);
	AddVariablePredef(m_shaderVar, luminanceMultiplier);
	AddVariablePredef(m_shaderVar, minLogLuminance);
	AddVariablePredef(m_shaderVar, tonemapMethod);
	AddVariablePredef(m_shaderVar, lensFlareDirtTextureUniform);
	AddVariablePredef(m_shaderVar, lensFlareGhostGradientTextureUniform);
	AddVariablePredef(m_shaderVar, lensFlareStarburstTextureUniform);
	AddVariablePredef(m_shaderVar, fogDensityUniform);
	AddVariablePredef(m_shaderVar, fogColorUniform);
	AddVariablePredef(m_shaderVar, billboardScaleUniform);
	AddVariablePredef(m_shaderVar, depthTypeUniform);
	AddVariablePredef(m_shaderVar, eyeAdaptionRateUniform);
	AddVariablePredef(m_shaderVar, eyeAdaptionIntBrightnessUniform);
	AddVariablePredef(m_shaderVar, AODataSetBuffer);
	AddVariablePredef(m_shaderVar, SSAOSampleBuffer);
	AddVariablePredef(m_shaderVar, CSMDataSetBuffer);
	AddVariablePredef(m_shaderVar, HDRSSBuffer);
	AddVariablePredef(m_shaderVar, atmScatParamBuffer);
	AddVariablePredef(m_shaderVar, lensFlareParametersBuffer);
	AddVariablePredef(m_shaderVar, testMatUniform);
	AddVariablePredef(m_shaderVar, testVecUniform);
	AddVariablePredef(m_shaderVar, testFloatUniform);
	AddVariablePredef(m_shaderVar, define_numOfCascades);
	AddVariablePredef(m_shaderVar, define_numOfPCFSamples);
	AddVariablePredef(m_shaderVar, define_parallaxMapping);
	AddVariablePredef(m_shaderVar, define_parallaxMappingMethod);
	AddVariablePredef(m_shaderVar, define_shadowMapping);
	AddVariablePredef(m_shaderVar, define_stochasticSampling);
	AddVariablePredef(m_shaderVar, define_stochasticSamplingSeamFix);

	// Texture variables
	AddVariablePredef(m_textureVar, default_texture);
	AddVariablePredef(m_textureVar, default_emissive_texture);
	AddVariablePredef(m_textureVar, default_height_texture);
	AddVariablePredef(m_textureVar, default_normal_texture);
	AddVariablePredef(m_textureVar, default_RMHA_texture);
	AddVariablePredef(m_textureVar, default_specular_intensity);
	AddVariablePredef(m_textureVar, default_specular_power);
	AddVariablePredef(m_textureVar, diffuse_texture_format);
	AddVariablePredef(m_textureVar, normal_texture_format);
	AddVariablePredef(m_textureVar, emissive_texture_format);
	AddVariablePredef(m_textureVar, roughness_texture_format);
	AddVariablePredef(m_textureVar, metalness_texture_format);
	AddVariablePredef(m_textureVar, height_texture_format);
	AddVariablePredef(m_textureVar, ambientOcclusion_texture_format);
	AddVariablePredef(m_textureVar, RMHAO_texture_format);
	AddVariablePredef(m_textureVar, gl_texture_anisotropy);
	AddVariablePredef(m_textureVar, gl_texture_magnification);
	AddVariablePredef(m_textureVar, gl_texture_minification);
	AddVariablePredef(m_textureVar, gl_texture_magnification_mipmap);
	AddVariablePredef(m_textureVar, gl_texture_minification_mipmap);
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

	for(int i = Properties::Null; i < Properties::NumberOfPropertyIDs; i++)
	{
		//Properties::PropertyNames[i] = GetString(static_cast<Properties::PropertyID>(i));
	}
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
			auto tableEntry = m_hashTable.find(singleWord);

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

	if(m_rendererVar.heightmap_combine_texture < MaterialType_Diffuse ||
	   m_rendererVar.heightmap_combine_texture >= MaterialType_NumOfTypes)
		m_rendererVar.heightmap_combine_texture = MaterialType_Normal;

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
	auto tableEntry = m_hashTable.find(p_name);

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